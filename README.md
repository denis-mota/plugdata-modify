# plugdata-modify

Fork do [plugdata](https://github.com/tim-koschke/plugdata) com suporte a imagens nos widgets gráficos (canvas, knob, toggle) ao compilar para DPF/LV2/VST2/JACK via [Heavy](https://github.com/Wasted-Audio/hvcc).

---

# plugdata-modify

Fork of [plugdata](https://github.com/tim-koschke/plugdata) with image support for GUI widgets (canvas, knob, toggle) when compiling to DPF/LV2/VST2/JACK via [Heavy](https://github.com/Wasted-Audio/hvcc).

---

## O que foi modificado / What was changed

### Generadores DPF (hvcc)

| Arquivo / File | Mudança / Change |
|---|---|
| `hvcc/generators/c2dpf/nanovg_render.py` | Embedding de imagens (canvas, knob, toggle) como C arrays; correção de glob para encontrar `.heavy.gui.json` correto |
| `hvcc/generators/c2dpf/c2dpf.py` | Filtro Jinja2 `c_id` para nomes C++ válidos |
| `hvcc/generators/c2dpf/templates/gui_objects.cpp` | Renderiza `image_data` via `setImageFromMemory()` para canvas, knob e toggle |
| `hvcc/generators/c2dpf/templates/HeavyDPF.cpp` | Correção de `capitalize` no `setOutputParameter` |
| `hvcc/generators/c2dpf/templates/HeavyDPF_NanoVG_UI.hpp` | Enum `ParamID` com filtro `c_id` para IDs de parâmetro |
| `hvcc/generators/ir2c/ControlExpr.py` | Fix regex para `\\\,` (backslash-comma) |
| `hvcc/generators/ir2c/SignalExpr.py` | Fix regex para `\\\,` |
| `hvcc/interpreters/pd2gui/PdGUIParser.py` | Parse de flags de imagem: canvas (linha 18), toggle (linha 19), knob (linha 36 com `-image`) |

### pdvg (lib/pdvg)

| Arquivo / File | Mudança / Change |
|---|---|
| `lib/pdvg/src/Knob.cpp` | Imagem gira com o valor do knob (rotação via `arcBegin`/`arcEnd`) |
| `lib/pdvg/src/Toggle.cpp` | Imagem com opacidade (1.0 ligado, 0.3 desligado) |

### Outros / Other

| Arquivo / File | Mudança / Change |
|---|---|
| `.gitignore` | Exclui `__pycache__/`, `*.pyc`, `plugdata/Plugins/` |
| `bin/Heavy` | Script de compilação Heavy (copiado do Toolchain) |

---

## Como usar / How to use

### 1. Compilar o patch / Compile the patch

```bash
./bin/Heavy caminho/para/patch.pd -o saida/ -nds1 --gui -g dpf
```

### 2. Adicionar imagens / Add images

No patch Pure Data, adicione o caminho da imagem como último argumento dos objetos:

**Canvas** (fundo):
```pd
#X obj 10 10 cnv 300 200 empty empty mycanvas 0 -10 0 4 #333333 #e1e1e1 #e1e1e1 0 0 /caminho/para/fundo.gif;
```

**Knob** (botão giratório):
```pd
#X obj 100 100 hsl 0 127 0 0 0 0 empty empty empty 0 -10 0 4 #e1e1e1 #333333 #333333 0 1 /caminho/para/knob.gif;
```

**Toggle** (liga/desliga):
```pd
#X obj 100 200 tgl 25 0 empty empty empty 0 -10 0 4 #e1e1e1 #333333 #333333 0 1 /caminho/para/toggle.gif;
```

As imagens devem estar no formato `.gif` (interno PNG) ou `.png`.

### 3. Compilar o plugin / Build the plugin

```bash
cd saida/
make
./bin/nome_do_patch
```

---

## Estrutura do repositório / Repository structure

```
plugdata-modify/
├── Abstractions/          # Abstrações Pure Data
├── bin/
│   └── Heavy              # Script de compilação Heavy
├── hvcc/
│   └── hvcc/
│       ├── generators/
│       │   ├── c2dpf/     # Gerador DPF (modificado)
│       │   └── ir2c/      # Expressões (regex fixes)
│       └── interpreters/
│           └── pd2gui/    # Parser GUI (image flags)
├── lib/
│   ├── dpf/               # DPF framework
│   └── pdvg/              # PD widgets (knob/toggle image support)
│       └── src/
│           ├── Knob.cpp   # Rotação de imagem
│           ├── Toggle.cpp # Opacidade de imagem
│           └── Canvas.cpp # Fundo com imagem
└── plugdata/              # Binários do plugdata
```

---

## Requisitos / Requirements

- Python 3.8+
- GCC/G++ com suporte C++17
- JACK (para áudio) ou ALSA
- make

---

## Créditos / Credits

- [plugdata](https://github.com/tim-koschke/plugdata) — Interface gráfica para Pure Data
- [Heavy](https://github.com/Wasted-Audio/hvcc) — Compilador Pure Data para C/C++
- [DPF](https://github.com/DISTRHO/DPF) — Framework de plugins de áudio
- [pdvg](https://github.com/Wasted-Audio/pdvg) — Widgets NanoVG para Pure Data

---

## Licença / License

Licença original do plugdata e hvcc (ISC). Verifique os arquivos individuais para detalhes.
