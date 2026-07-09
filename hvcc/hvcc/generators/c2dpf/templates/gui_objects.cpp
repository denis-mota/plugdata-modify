    {%- for object in gui_objects -%}
        {%- if object.type == 'canvas' %}
    // canvas
    {{object.id}} = new PDCanvas({{parent}});
    {{object.id}}->setSize({{object.size.x}} * scaleFactor, {{object.size.y}} * scaleFactor);
    {{object.id}}->setAbsolutePos({{object.position.x}} * scaleFactor, {{object.position.y}} * scaleFactor);
    {{object.id}}->setColors(nvgRGB{{object.bg_color.as_rgb_tuple()}});
            {%- if object.label != None %}
    {{object.id}}->setLabel("{{object.label.text}}", nvgRGB{{object.label.color.as_rgb_tuple()}}, {{object.label.position.x}} * scaleFactor, {{object.label.position.y}} * scaleFactor, {{object.label.font_size}} * scaleFactor);
            {%- endif %}
            {%- if object.id in image_data_map %}
    {%- set img_data, img_var = image_data_map[object.id] %}
    static const unsigned char {{object.id}}_img[] = {
        {{- img_data | join(', ') }}
    };
    {{object.id}}->setImageFromMemory({{object.id}}_img, sizeof({{object.id}}_img));
            {%- endif %}
        {%- elif object.type == 'comment' %}
    // comment
    {{object.id}} = new PDComment({{parent}});
    std::string {{object.id}}String = "{{object.text}}";
    {{object.id}}->setText({{object.id}}String);
    {{object.id}}->setFontSize(15 * scaleFactor);
    {{object.id}}->setAbsolutePos({{object.position.x}} * scaleFactor, {{object.position.y}} * scaleFactor);
            {%- if object.width != None %}
    {{object.id}}->setSize((7.4 * {{object.width}}) * scaleFactor, 15 * scaleFactor);
            {%- else %}
    {{object.id}}->setSize((7.4 * {{object.id}}String.length()) * scaleFactor, 15 * scaleFactor);
            {%- endif -%}
        {%- elif object.type == 'bang' %}
    // bang
    {{object.parameter|c_id}} = new PDBang({{parent}}, this);
    {{object.parameter|c_id}}->setId(k{{object.parameter|c_id|capitalize}});
    {{object.parameter|c_id}}->setSize({{object.size.x}} * scaleFactor, {{object.size.y}} * scaleFactor);
    {{object.parameter|c_id}}->setAbsolutePos({{object.position.x}} * scaleFactor, {{object.position.y}} * scaleFactor);
    {{object.parameter|c_id}}->setColors(
        nvgRGB{{object.bg_color.as_rgb_tuple()}},
        nvgRGB{{object.fg_color.as_rgb_tuple()}}
    );
    {{object.parameter|c_id}}->setInterval({{object.flash_time}});
            {%- if object.label != None %}
    {{object.parameter|c_id}}->setLabel("{{object.label.text}}", nvgRGB{{object.label.color.as_rgb_tuple()}}, {{object.label.position.x}} * scaleFactor, {{object.label.position.y}} * scaleFactor, {{object.label.font_size}} * scaleFactor);
            {%- endif %}
    {{parent}}->addManagedChild({{object.parameter|c_id}});
        {%- elif object.type == 'toggle' %}
    // toggle
    {{object.parameter|c_id}} = new PDToggle({{parent}}, this);
    {{object.parameter|c_id}}->setId(k{{object.parameter|c_id|capitalize}});
    {{object.parameter|c_id}}->setSize({{object.size.x}} * scaleFactor, {{object.size.y}} * scaleFactor);
    {{object.parameter|c_id}}->setAbsolutePos({{object.position.x}} * scaleFactor, {{object.position.y}} * scaleFactor);
    {{object.parameter|c_id}}->setColors(
        nvgRGB{{object.bg_color.as_rgb_tuple()}},
        nvgRGB{{object.fg_color.as_rgb_tuple()}}
    );
            {%- if object.parameter in image_data_map %}
    {%- set img_data, img_var = image_data_map[object.parameter] %}
    static const unsigned char {{object.parameter|c_id}}_img[] = {
        {{- img_data | join(', ') }}
    };
    {{object.parameter|c_id}}->setImageFromMemory({{object.parameter|c_id}}_img, sizeof({{object.parameter|c_id}}_img));
            {%- endif %}
    {{parent}}->addManagedChild({{object.parameter|c_id}});
        {%- elif object.type in ['vradio', 'hradio'] %}
    // {{object.type}}
    {{object.parameter|c_id}} = new PDRadio({{parent}}, this);
    {{object.parameter|c_id}}->setId(k{{object.parameter|c_id|capitalize}});
    {{object.parameter|c_id}}->setSize({{object.size.x}} * scaleFactor, {{object.size.y}} * scaleFactor);
    {{object.parameter|c_id}}->setAbsolutePos({{object.position.x}} * scaleFactor, {{object.position.y}} * scaleFactor);
    {{object.parameter|c_id}}->setStep({{object.options}});
            {%- if object.type == "hradio" %}
    {{object.parameter|c_id}}->setHorizontal();
            {%- endif %}
    {{object.parameter|c_id}}->setColors(
        nvgRGB{{object.bg_color.as_rgb_tuple()}},
        nvgRGB{{object.fg_color.as_rgb_tuple()}}
    );
                {%- if object.label != None %}
    {{object.parameter|c_id}}->setLabel("{{object.label.text}}", nvgRGB{{object.label.color.as_rgb_tuple()}}, {{object.label.position.x}} * scaleFactor, {{object.label.position.y}} * scaleFactor, {{object.label.font_size}} * scaleFactor);
                {%- endif %}
    {{parent}}->addManagedChild({{object.parameter|c_id}});
        {%- elif object.type in ['vslider', 'hslider'] %}
    // {{object.type}}
    {{object.parameter|c_id}} = new PDSlider({{parent}}, this);
    {{object.parameter|c_id}}->setId(k{{object.parameter|c_id|capitalize}});
    {{object.parameter|c_id}}->setSize({{object.size.x}} * scaleFactor, {{object.size.y}} * scaleFactor);
    {{object.parameter|c_id}}->setAbsolutePos({{object.position.x}} * scaleFactor, {{object.position.y}} * scaleFactor);
    {{object.parameter|c_id}}->setSliderArea(0, 0, {{object.size.x}} * scaleFactor, {{object.size.y}} * scaleFactor);
            {%- if object.type == "vslider" %}
    {{object.parameter|c_id}}->setStartPos(0, {{object.size.y}} * scaleFactor);
    {{object.parameter|c_id}}->setEndPos(0, 0);
    {{object.parameter|c_id}}->setInverted(true);
            {%- elif object.type == "hslider" %}
    {{object.parameter|c_id}}->setStartPos(0, 0);
    {{object.parameter|c_id}}->setEndPos({{object.size.x}} * scaleFactor, 0);
    {{object.parameter|c_id}}->setHorizontal();
            {%- endif %}
    {{object.parameter|c_id}}->setRange({{object.min}}f, {{object.max}}f);
        {%- for k, v in receivers + senders %}
            {%- if v.display == object.parameter %}
    {{object.parameter|c_id}}->setDefault({{v.attributes.default}}f);
            {%- endif %}
        {%- endfor %}
    {{object.parameter|c_id}}->setUsingLogScale({{object.logarithmic|lower}});
    {{object.parameter|c_id}}->setSteadyOnClick({{object.steady|lower}});
    {{object.parameter|c_id}}->setColors(
        nvgRGB{{object.bg_color.as_rgb_tuple()}},
        nvgRGB{{object.fg_color.as_rgb_tuple()}}
    );
                {%- if object.label != None %}
    {{object.parameter|c_id}}->setLabel("{{object.label.text}}", nvgRGB{{object.label.color.as_rgb_tuple()}}, {{object.label.position.x}} * scaleFactor, {{object.label.position.y}} * scaleFactor, {{object.label.font_size}} * scaleFactor);
                {%- endif %}
    {{parent}}->addManagedChild({{object.parameter|c_id}});
        {%- elif object.type == 'knob' %}
    // knob
    {{object.parameter|c_id}} = new PDKnob({{parent}}, this);
    {{object.parameter|c_id}}->setId(k{{object.parameter|c_id|capitalize}});
    {{object.parameter|c_id}}->setSize({{object.size.x}} * scaleFactor, {{object.size.y}} * scaleFactor);
    {{object.parameter|c_id}}->setAbsolutePos({{object.position.x}} * scaleFactor, {{object.position.y}} * scaleFactor);
    {{object.parameter|c_id}}->setKnobArea(0.0f, 0.0f, {{object.size.x}} * scaleFactor, {{object.size.y}} * scaleFactor);
    {{object.parameter|c_id}}->setRange({{object.min}}f, {{object.max}}f);
        {%- for k, v in receivers + senders %}
            {%- if v.display == object.parameter %}
    {{object.parameter|c_id}}->setDefault({{v.attributes.default}}f);
            {%- endif %}
        {%- endfor %}
    {{object.parameter|c_id}}->setShowArc({{object.arc_show|lower}});
    {{object.parameter|c_id}}->setAngular({{object.ang_range}}, {{object.ang_offset}});
    {{object.parameter|c_id}}->setDrawSquare({{object.square|lower}});
    {{object.parameter|c_id}}->setShowTicks({{object.ticks|lower}});
    {{object.parameter|c_id}}->setSteps({{object.steps}});
    {{object.parameter|c_id}}->setJumpOnClick({{object.jump|lower}});
    {{object.parameter|c_id}}->setDiscrete({{object.discrete|lower}});
    {{object.parameter|c_id}}->setUsingLogScale(PDKnobEventHandler::LogMode::{{object.log_mode|upper}});
    {{object.parameter|c_id}}->setColors(
        nvgRGB{{object.bg_color.as_rgb_tuple()}},
        nvgRGB{{object.fg_color.as_rgb_tuple()}},
        nvgRGB{{object.arc_color.as_rgb_tuple()}}
    );
        {%- if object.label_size > 0 %}
    {{object.parameter|c_id}}->setLabelStyle({{object.label_pos.x}} * scaleFactor, {{object.label_pos.y}} * scaleFactor, {{object.label_size}} * scaleFactor);
    {{object.parameter|c_id}}->setShowLabel(LabelShow::{{object.label_show.name|upper}});
        {%- endif %}
            {%- if object.parameter in image_data_map %}
    {%- set img_data, img_var = image_data_map[object.parameter] %}
    static const unsigned char {{object.parameter|c_id}}_img[] = {
        {{- img_data | join(', ') }}
    };
    {{object.parameter|c_id}}->setImageFromMemory({{object.parameter|c_id}}_img, sizeof({{object.parameter|c_id}}_img));
            {%- endif %}
    {{parent}}->addManagedChild({{object.parameter|c_id}});
        {%- elif object.type == 'number' %}
    // number
    {{object.parameter|c_id}} = new PDNumber({{parent}}, this);
    {{object.parameter|c_id}}->setId(k{{object.parameter|c_id|capitalize}});
    {{object.parameter|c_id}}->setSize({{object.size.x}} * scaleFactor, {{object.size.y}} * scaleFactor);
    {{object.parameter|c_id}}->setAbsolutePos({{object.position.x}} * scaleFactor, {{object.position.y}} * scaleFactor);
    {{object.parameter|c_id}}->setRange({{object.min}}f, {{object.max}}f);
        {%- for k, v in receivers + senders %}
            {%- if v.display == object.parameter %}
    {{object.parameter|c_id}}->setDefault({{v.attributes.default}}f);
            {%- endif %}
        {%- endfor %}
    {{object.parameter|c_id}}->setLogMode({{object.log_mode|lower}});
    {{object.parameter|c_id}}->setColors(
        nvgRGB{{object.bg_color.as_rgb_tuple()}},
        nvgRGB{{object.fg_color.as_rgb_tuple()}}
    );
                {%- if object.label != None %}
    {{object.parameter|c_id}}->setLabel("{{object.label.text}}", nvgRGB{{object.label.color.as_rgb_tuple()}}, {{object.label.position.x}} * scaleFactor, {{object.label.position.y}} * scaleFactor, {{object.label.font_size}} * scaleFactor);
                {%- endif %}
    {{parent}}->addManagedChild({{object.parameter|c_id}});
        {%- elif object.type == 'float' %}
    // float
    {{object.parameter|c_id}} = new PDFloat({{parent}}, this);
    {{object.parameter|c_id}}->setId(k{{object.parameter|c_id|capitalize}});
    {{object.parameter|c_id}}->setSize({{object.size.x}} * scaleFactor, ({{object.size.y}} * 1.5f) * scaleFactor);
    {{object.parameter|c_id}}->setAbsolutePos({{object.position.x}} * scaleFactor, {{object.position.y}} * scaleFactor);
    {{object.parameter|c_id}}->setRange({{object.min}}f, {{object.max}}f);
        {%- for k, v in receivers + senders %}
            {%- if v.display == object.parameter %}
    {{object.parameter|c_id}}->setDefault({{v.attributes.default}}f);
            {%- endif %}
        {%- endfor %}
    {{object.parameter|c_id}}->setLabel("{{object.label_text}}", ({{object.font_height}} + 2) * scaleFactor, LabelPos::{{object.label_pos.name|capitalize}});
    {{parent}}->addManagedChild({{object.parameter|c_id}});
        {%- endif %}
    {%- endfor %}
