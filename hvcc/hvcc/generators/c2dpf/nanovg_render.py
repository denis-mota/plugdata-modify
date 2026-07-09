import json

from pathlib import Path

import jinja2

from hvcc.types.GUI import Canvas, Comment, GraphRoot, Graph, GUIObjects, Knob, Toggle


def open_gui_json(
    patch_name: str,
    c_src_dir: Path,
) -> GraphRoot:

    # load GUI from json file (glob to handle name mismatch between pd2gui output and c2dpf input)
    ir_dir = Path(c_src_dir, "../ir/")
    gui_json_files = list(ir_dir.glob("*.heavy.gui.json"))
    if not gui_json_files:
        raise FileNotFoundError(f"No GUI JSON found in {ir_dir}")
    # prefer the file matching patch_name, else use the most recent
    gui_json_path = None
    for f in gui_json_files:
        if f.stem.startswith(patch_name):
            gui_json_path = f
            break
    if gui_json_path is None:
        gui_json_path = max(gui_json_files, key=lambda f: f.stat().st_mtime)
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


def ensure_gui_params(recv_list: list, gui_json: GraphRoot):
    """ Ensure GUI widget parameters exist in recv_list and have correct attributes.
        Adds missing params. Updates existing toggle/radio to type="bool" for setDown().
    """
    existing_params = {v.display: v for _, v in recv_list}

    from hvcc.types.IR import IRSendMessage
    from hvcc.types.GUI import Toggle, VRadio, HRadio

    def is_bool_type(widget_type):
        if isinstance(widget_type, type) and issubclass(widget_type, (Toggle, VRadio, HRadio)):
            return True
        if widget_type in ("toggle", "vradio", "hradio"):
            return True
        return False

    def get_param_type(widget_type):
        if isinstance(widget_type, type):
            if issubclass(widget_type, Toggle):
                return "toggle"
            if issubclass(widget_type, (VRadio, HRadio)):
                return "radio"
        if widget_type == "toggle":
            return "toggle"
        if widget_type in ("vradio", "hradio"):
            return "radio"
        return "knob"

    def ensure_recv(param_name: str, min_val: float = 0.0, max_val: float = 1.0, widget_type: str = "knob"):
        if not param_name:
            return
        param_type = get_param_type(widget_type)
        if param_name in existing_params:
            # update existing entry: set param type for correct UI method call
            existing_params[param_name].attributes = {
                **(existing_params[param_name].attributes or {}),
                "type": param_type
            }
        else:
            attrs = {"min": min_val, "max": max_val, "default": 0.0, "type": param_type}
            recv_list.append((param_name, IRSendMessage(
                id=param_name,
                onMessage=[],
                display=param_name,
                attributes=attrs,
                extern=""  # real param, send to Heavy
            )))
            existing_params[param_name] = recv_list[-1][1]

    def collect_params(objects):
        for w in objects:
            if isinstance(w, (Canvas, Comment)):
                continue
            if hasattr(w, 'parameter') and w.parameter:
                mn = getattr(w, 'min', 0.0) or 0.0
                mx = getattr(w, 'max', 1.0) or 1.0
                ensure_recv(w.parameter, min_val=mn, max_val=mx, widget_type=w.type)

    collect_params(gui_json.objects)
    for g in gui_json.graphs:
        collect_params(g.objects)


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

    # ensure GUI widget types are set correctly for parameterChanged callbacks
    ensure_gui_params(recv_list, gui_json)

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
