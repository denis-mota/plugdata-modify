import json

from pathlib import Path

import jinja2

from hvcc.types.GUI import Canvas, Comment, GraphRoot, Graph, GUIObjects, Knob, Toggle


def open_gui_json(
    patch_name: str,
    c_src_dir: Path,
) -> GraphRoot:

    # load GUI from json file
    gui_json_path = Path(c_src_dir, "../ir/", f"{patch_name}.heavy.gui.json")
    with open(gui_json_path, "r") as f:
        gui_json = GraphRoot(**json.load(f))

    return gui_json


def load_image_bytes(image_path: Path) -> tuple[list[str], str] | None:
    """Load raw image file bytes for embedding.
    Returns (hex_strings, variable_name) or None if file not found."""
    if not image_path.is_file():
        return None

    raw = image_path.read_bytes()
    hex_strings = [f"0x{b:02x}" for b in raw]
    # create a valid C variable name from the filename
    var_name = image_path.stem
    var_name = "".join(c if c.isalnum() else "_" for c in var_name)
    return hex_strings, var_name


def nanovg_render(
    patch_name: str,
    c_src_dir: Path,
    env: jinja2.Environment,
    recv_list: list,
    send_list: list
) -> tuple[
    GraphRoot,
    dict[str, list[str]],
    list[str]
]:
    """ Generate nanovg components from the GUI json
    """
    gui_json = open_gui_json(patch_name, c_src_dir)

    # widget overview
    widgets: dict[str, list[str]] = {
        "graph": [],
        "canvas": [],
        "comment": [],
        "bang": [],
        "toggle": [],
        "vradio": [],
        "hradio": [],
        "vslider": [],
        "hslider": [],
        "knob": [],
        "number": [],
        "float": []
    }

    # render gui objects
    gui_objects_render = []

    def generate_gui_objects(graphs: list[Graph], objects: list[GUIObjects], parent: str):
        for w in objects:
            widgets[w.type].append(w.id if isinstance(w, (Canvas, Comment)) else w.parameter)

        # resolve image paths for canvas, knob and toggle objects
        image_data_map = {}
        for w in objects:
            img_path = None
            if isinstance(w, Canvas) and w.image_path:
                img_path = w.image_path
            elif isinstance(w, Knob) and w.image_path:
                img_path = w.image_path
            elif isinstance(w, Toggle) and w.image_path:
                img_path = w.image_path

            if img_path:
                # resolve relative to the c_src_dir's parent (the output dir)
                resolved = c_src_dir.parent / img_path
                result = load_image_bytes(resolved)
                if result is not None:
                    raw_bytes, var_name = result
                    obj_id = w.id if isinstance(w, Canvas) else w.parameter
                    image_data_map[obj_id] = (raw_bytes, var_name)

        gui_objects_render.append(env.get_template("gui_objects.cpp").render(
            parent=parent,
            gui_objects=objects,
            receivers=recv_list,
            senders=send_list,
            image_data_map=image_data_map
        ))

        for graph in graphs:
            widgets["graph"].append(graph.id)

            gui_objects_render.append(
                f"""
    // subpatch
    {graph.id} = new PDSubpatch({parent});
    {graph.id}->setSize({graph.gop_size.x} * scaleFactor, {graph.gop_size.y} * scaleFactor);
    {graph.id}->setAbsolutePos({graph.position.x} * scaleFactor, {graph.position.y} * scaleFactor);
    {parent}->addManagedChild({graph.id});
                """
                    )
            generate_gui_objects(graph.graphs, graph.objects, graph.id)

    generate_gui_objects(gui_json.graphs, gui_json.objects, "mainPatch")

    return gui_json, widgets, gui_objects_render
