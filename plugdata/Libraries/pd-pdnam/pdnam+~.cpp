// PDNAM
// Copyright (C) 2026 Wasted Audio
//
// SPDX-License-Identifier: MIT

#include "m_pd.h"
#include "g_canvas.h"
#include "NeuralAudio/NeuralModel.h"
#include "nlohmann/json.hpp"
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <filesystem>

static t_class *pdnam_plus_tilde_class;

typedef struct _pdnam_plus_tilde {
    t_object  x_obj;
    t_sample f;
    t_canvas* canvas;
    NeuralAudio::NeuralModel* model;
} t_pdnam_plus_tilde;

static void pdnam_plus_tilde_load(t_pdnam_plus_tilde *x, t_symbol *model_path)
{
    if (x->model) {
        delete x->model;
        x->model = nullptr;
    }

    if (model_path == &s_) return;

    char buf[MAXPDSTRING];
    canvas_makefilename(x->canvas, model_path->s_name, buf, MAXPDSTRING);
    std::filesystem::path path(buf);

    if (!std::filesystem::exists(path) || !std::filesystem::is_regular_file(path)) {
        t_symbol *dir = canvas_getdir(x->canvas);
        snprintf(buf, MAXPDSTRING, "%s/%s", dir->s_name, model_path->s_name);
        path = std::filesystem::path(buf);
    }

    if (!std::filesystem::exists(path) || !std::filesystem::is_regular_file(path)) {
        pd_error(x, "pdnam+~: model file not found: %s", model_path->s_name);
        return;
    }

    try {
        std::ifstream fileStream(path, std::ifstream::binary);
        nlohmann::json modelJson;
        fileStream >> modelJson;
        fileStream.close();

        std::string arch = modelJson.value("architecture", "");

        if (arch == "SlimmableContainer" && modelJson.contains("config")) {
            auto& config = modelJson["config"];
            if (config.contains("submodels") && config["submodels"].is_array() && !config["submodels"].empty()) {
                float bestMax = -1.0f;
                nlohmann::json bestModel;

                for (auto& sm : config["submodels"]) {
                    if (sm.contains("max_value") && sm.contains("model")) {
                        float mv = sm["max_value"].get<float>();
                        if (mv > bestMax) {
                            bestMax = mv;
                            bestModel = sm["model"];
                        }
                    }
                }

                if (!bestModel.is_null() && bestModel.contains("weights") && !bestModel["weights"].empty()) {
                    post("pdnam+~: SlimmableContainer -> %s (%zu weights, gain %.1f)",
                         bestModel.value("architecture", "?").c_str(),
                         bestModel["weights"].size(),
                         bestMax);

                    std::string dump = bestModel.dump();
                    std::stringstream ss(dump);
                    x->model = NeuralAudio::NeuralModel::CreateFromStream(ss, path.extension());

                    if (!x->model) {
                        std::string sub_arch = bestModel.value("architecture", "?");
                        pd_error(x, "pdnam+~: %s submodel not supported by NeuralAudio", sub_arch.c_str());
                        pd_error(x, "pdnam+~: re-export model in standard NAM format (not SlimmableContainer)");
                    }
                } else {
                    pd_error(x, "pdnam+~: SlimmableContainer has no valid submodels");
                }
            } else {
                pd_error(x, "pdnam+~: SlimmableContainer missing submodels");
            }
        } else {
            std::ifstream freshStream(path, std::ifstream::binary);
            x->model = NeuralAudio::NeuralModel::CreateFromStream(freshStream, path.extension());
        }

        if (x->model) {
            post("pdnam+~: loaded model %s", model_path->s_name);
        } else {
            pd_error(x, "pdnam+~: failed to load model %s", model_path->s_name);
        }
    } catch (const std::exception& e) {
        pd_error(x, "pdnam+~: exception loading model: %s", e.what());
    } catch (...) {
        pd_error(x, "pdnam+~: unknown exception loading model");
    }
}

void *pdnam_plus_tilde_new(t_symbol *s, int argc, t_atom *argv)
{
    t_pdnam_plus_tilde *x = (t_pdnam_plus_tilde *)pd_new(pdnam_plus_tilde_class);
    x->canvas = canvas_getcurrent();
    x->model = nullptr;

    t_symbol* model_path = (argc >= 1 && argv[0].a_type == A_SYMBOL)
        ? atom_getsymbol(&argv[0])
        : &s_;

    pdnam_plus_tilde_load(x, model_path);

    outlet_new(&x->x_obj, &s_signal);

    return (void *)x;
}

t_int *pdnam_plus_tilde_perform(t_int *w)
{
    t_pdnam_plus_tilde *x = (t_pdnam_plus_tilde *)(w[1]);
    t_sample  *in = (t_sample *)(w[2]);
    t_sample  *out = (t_sample *)(w[3]);
    int n = (int)(w[4]);

    if (x->model) {
        x->model->Process(in, out, n);
    } else {
        while (n--) {
            *out++ = *in++;
        }
    }

    return (w+5);
}

void pdnam_plus_tilde_dsp(t_pdnam_plus_tilde *x, t_signal **sp)
{
    dsp_add(pdnam_plus_tilde_perform, 4, x,
            sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

void pdnam_plus_tilde_free(t_pdnam_plus_tilde *x)
{
    if (x->model) delete x->model;
}

static void pdnam_plus_tilde_print(t_pdnam_plus_tilde *x)
{
    if (x->model) {
        post("pdnam+~: model loaded, processing active");
    } else {
        post("pdnam+~: NO MODEL LOADED (passthrough mode)");
    }
}

// ponytail: one-shot diagnostic
static void pdnam_plus_tilde_debug(t_pdnam_plus_tilde *x)
{
    post("pdnam+~: model_ptr=%p", (void*)x->model);
    if (x->model) {
        float in_val = 0.5f, out_val = 0.f;
        x->model->Process(&in_val, &out_val, 1);
        post("pdnam+~: test Process: in=0.5 out=%f", out_val);
    }
}

extern "C" void setup_pdnam0x2b_tilde(void) {
    pdnam_plus_tilde_class = class_new(gensym("pdnam+~"),
        (t_newmethod)pdnam_plus_tilde_new,
        (t_method)pdnam_plus_tilde_free, sizeof(t_pdnam_plus_tilde),
        CLASS_DEFAULT, A_GIMME, 0);

    class_addmethod(pdnam_plus_tilde_class,
        (t_method)pdnam_plus_tilde_dsp, gensym("dsp"), A_CANT, 0);

    class_addmethod(pdnam_plus_tilde_class,
        (t_method)pdnam_plus_tilde_load, gensym("set"), A_SYMBOL, 0);

    class_addmethod(pdnam_plus_tilde_class,
        (t_method)pdnam_plus_tilde_print, gensym("print"), A_NULL, 0);

    class_addmethod(pdnam_plus_tilde_class,
        (t_method)pdnam_plus_tilde_debug, gensym("debug"), A_NULL, 0);


    CLASS_MAINSIGNALIN(pdnam_plus_tilde_class, t_pdnam_plus_tilde, f);
}
