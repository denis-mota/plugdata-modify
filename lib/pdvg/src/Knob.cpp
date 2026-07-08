/*
 * Copyright (C) 2026 Wasted Audio
 * SPDX-License-Identifier: ISC
*/

#include "nanovg.h"
#include <cctype>
#include <cmath>
#include <cstring>

#include "Common.hpp"
#include "Knob.hpp"

START_NAMESPACE_DISTRHO

PDKnob::PDKnob(NanoSubWidget *parent, PDKnobEventHandler::Callback *const cb)
    : PDWidget(parent),
      PDKnobEventHandler(this)
{
    PDKnobEventHandler::setCallback(cb);

    using namespace InterRegular;
    NanoVG::FontId interId = createFontFromMemory("inter", (const uchar *)InterRegularData, InterRegularDataSize, 0);
    fFontId = interId;
}

PDKnob::~PDKnob()
{
    NVGcontext* nvg = getContext();
    if (nvg != nullptr && imageHandle >= 0)
        nvgDeleteImage(nvg, imageHandle);
}

void PDKnob::drawTicks(NVGcontext* nvg, DGL::Rectangle<float> pBounds, float startAngle, float endAngle, float tickWidth)
{
    auto const centerX = pBounds.getX() + pBounds.getWidth() / 2.0f;
    auto const centerY = pBounds.getY() + pBounds.getHeight() / 2.0f;
    auto const radius = pBounds.getWidth() * 0.5f * 1.05f;

    // Calculate the angle between each tick
    float const angleIncrement = (endAngle - startAngle) / static_cast<float>(jmax(steps - 1, 1));

    // Position each tick around the larger circle
    float const tickRadius = tickWidth * 0.33f;
    for (int i = 0; i < steps; ++i) {
        float const angle = startAngle + i * angleIncrement;
        float const x = centerX + radius * std::cos(angle);
        float const y = centerY + radius * std::sin(angle);

        // Draw the tick at this position
        nvgBeginPath(nvg);
        nvgCircle(nvg, x, y, tickRadius);
        nvgFillColor(nvg, fgColor);
        nvgFill(nvg);
    }
}

void PDKnob::drawKnob(NVGcontext* nvg, DGL::Rectangle<float> pBounds)
{
    // const float scaleFactor = getTopLevelWidget()->getScaleFactor();
    const DGL::Rectangle<float> bounds = reduceRectangle(pBounds, getWidth() * 0.14f);
    auto const lineThickness = std::max(bounds.getWidth() * 0.09f, 1.5f);

    auto const knobNormValue = getNormalizedValue();
    auto startAngle = arcBegin - NVG_PI * 0.5f;
    auto const endAngle = arcEnd - NVG_PI * 0.5f;

    auto const angle = jmap<float>(knobNormValue, startAngle, endAngle);
    auto const center = jmap<double>(arcStart, startAngle, endAngle);

    startAngle = clamp(startAngle, endAngle + NVG_PI * 2.0f, endAngle - NVG_PI * 2.0f);

    auto boundsCenterX = bounds.getX() + bounds.getWidth() / 2.0f;
    auto boundsCenterY = bounds.getY() + bounds.getHeight() / 2.0f;

    if (showArc) {
        auto const arcBounds = reduceRectangle(bounds, lineThickness);
        auto const arcRadius = arcBounds.getWidth() * 0.5;
        auto const arcWidth = (arcRadius - lineThickness) / arcRadius;

        nvgBeginPath(nvg);
        nvgArc(nvg, boundsCenterX, boundsCenterY, arcRadius, startAngle, endAngle, NVG_HOLE);
        nvgStrokeWidth(nvg, arcWidth * lineThickness);
        nvgStrokeColor(nvg, arcColor);
        nvgStroke(nvg);

        nvgBeginPath(nvg);
        if (center < angle) {
            nvgArc(nvg, boundsCenterX, boundsCenterY, arcRadius, center, angle, NVG_HOLE);
        } else {
            nvgArc(nvg, boundsCenterX, boundsCenterY, arcRadius, angle, center, NVG_HOLE);
        }
        nvgStrokeColor(nvg, fgColor);
        nvgStrokeWidth(nvg, arcWidth * lineThickness);
        nvgStroke(nvg);
    }

    float const wiperX = boundsCenterX + bounds.getWidth() * 0.4f * std::cos(angle);
    float const wiperY = boundsCenterY + bounds.getWidth() * 0.4f * std::sin(angle);

    // draw wiper
    nvgBeginPath(nvg);
    nvgMoveTo(nvg, boundsCenterX, boundsCenterY);
    nvgLineTo(nvg, wiperX, wiperY);
    nvgStrokeWidth(nvg, lineThickness);
    nvgStrokeColor(nvg, fgColor);
    nvgLineCap(nvg, NVG_ROUND);
    nvgStroke(nvg);

    if (showTicks)
        drawTicks(nvg, bounds, startAngle, endAngle, lineThickness);
}

void PDKnob::onNanoDisplay()
{
    // const float scaleFactor = getTopLevelWidget()->getScaleFactor();
    const DGL::Rectangle<float> b(0.0f, 0.0f, getWidth(), getHeight());

    NVGcontext* nvg = getContext();

    if (drawSquare) {
        auto const lineThickness = std::max(b.getWidth() * 0.03f, 1.0f);

        drawRoundedRect(nvg, b.getX(), b.getY(), b.getWidth(), b.getHeight(), bgColor, Colors::outColor, Corners::objectCornerRadius);

        if (imageHandle >= 0)
        {
            const DGL::Rectangle<float> area = reduceRectangle(b, 2.0f);
            NVGpaint imgPaint = nvgImagePattern(nvg,
                area.getX(), area.getY(),
                area.getWidth(), area.getHeight(),
                0.0f, imageHandle, 1.0f);
            nvgBeginPath(nvg);
            nvgRoundedRect(nvg, area.getX(), area.getY(),
                           area.getWidth(), area.getHeight(),
                           Corners::objectCornerRadius);
            nvgFillPaint(nvg, imgPaint);
            nvgFill(nvg);
        }
        else
        {
            if (!showArc) {
                nvgBeginPath(nvg);
                nvgStrokeWidth(nvg, lineThickness);
                nvgStrokeColor(nvg, arcColor);
                nvgCircle(nvg, b.getX() + b.getWidth() / 2.0f, b.getY() + b.getHeight() / 2.0f, b.getWidth() / 2.7f);
                nvgStroke(nvg);
            }

            drawKnob(nvg, b);
        }
    } else {
        auto circleBounds = reduceRectangle(b, b.getWidth() * 0.13f);
        auto const lineThickness = std::max(circleBounds.getWidth() * 0.07f, 1.5f);
        circleBounds = reduceRectangle(circleBounds, lineThickness - 0.5f);

        auto circleCenterX = circleBounds.getX() + circleBounds.getWidth() / 2.0f;
        auto circleCenterY = circleBounds.getY() + circleBounds.getHeight() / 2.0f;

        float constexpr localScale = 1.3f;
        auto const originalCentre = DGL::Rectangle<float>(circleCenterX, circleCenterY, circleBounds.getWidth() / 2.0f, circleBounds.getHeight() / 2.0f);
        float const scaleOffsetX = originalCentre.getX() * (1.0f - localScale);
        float const scaleOffsetY = originalCentre.getY() * (1.0f - localScale);

        nvgSave(nvg);
        nvgTranslate(nvg, scaleOffsetX, scaleOffsetY);
        nvgScale(nvg, localScale, localScale);

        nvgFillColor(nvg, bgColor);
        nvgBeginPath(nvg);
        nvgCircle(nvg, circleCenterX, circleCenterY, circleBounds.getWidth() / 2.0f);
        nvgFill(nvg);

        nvgStrokeColor(nvg, Colors::outColor);
        nvgStrokeWidth(nvg, 1.0f);
        nvgStroke(nvg);

        if (imageHandle >= 0)
        {
            const DGL::Rectangle<float> area = reduceRectangle(b, 2.0f);
            NVGpaint imgPaint = nvgImagePattern(nvg,
                area.getX(), area.getY(),
                area.getWidth(), area.getHeight(),
                0.0f, imageHandle, 1.0f);
            nvgBeginPath(nvg);
            nvgCircle(nvg, circleCenterX, circleCenterY, circleBounds.getWidth() / 2.0f);
            nvgFillPaint(nvg, imgPaint);
            nvgFill(nvg);
        }
        else
        {
            drawKnob(nvg, b);
        }

        nvgRestore(nvg);
    }


    if (isTyping)
    {
        // Show the typed text with a cursor directly via NanoVG
        const float scaleFactor = getTopLevelWidget()->getScaleFactor();
        nvgFontFaceId(nvg, fFontId);
        nvgFontSize(nvg, getHeight() * 0.5f);
        nvgFillColor(nvg, Colors::cnvTextColor);
        nvgTextAlign(nvg, NVG_ALIGN_MIDDLE | NVG_ALIGN_LEFT);

        std::string displayText = typingText + "|";
        nvgText(nvg, b.getX() + 4.0f * scaleFactor, b.getY() + b.getHeight() / 2.0f,
                displayText.c_str(), nullptr);
    }
    else if (label != nullptr)
    {
        if (showLabel == ALWAYS || (showLabel == ACTIVE && isActive))
        {
            char buffer[32];
            std::snprintf(buffer, sizeof(buffer), "%.2f", getValue());
            std::string result(buffer);
            label->setText(result);
        }
        else if (showLabel == ACTIVE)
        {
            label->setText(" ");
        }
    }
}

bool PDKnob::onMouse(const MouseEvent &ev)
{
    if (ev.press && ev.button == 1)
    {
        const Point<int> screen = getScreenPos();
        const DGL::Rectangle<float> bounds(0.0f, 0.0f, getWidth(), getHeight());
        const bool inside = bounds.contains(ev.pos.getX() - screen.getX(), ev.pos.getY() - screen.getY());

        if (inside && !isActive)
        {
            isActive = true;
            if (showLabel == TYPING && !isTyping)
            {
                isTyping = true;
                typingText.clear();
            }
            repaint();
        }
        else if (!inside && isActive)
        {
            isActive = false;
            if (isTyping)
            {
                // Commit typed value before deactivating
                if (!typingText.empty())
                {
                    float typedValue = 0.0f;
                    if (std::sscanf(typingText.c_str(), "%f", &typedValue) == 1)
                    {
                        setValue(typedValue, true);
                    }
                }
                isTyping = false;
                typingText.clear();
            }
            repaint();
        }
    }

    if (isTyping)
        return false; // Don't pass mouse events to knob handler while typing

    return PDKnobEventHandler::mouseEvent(ev);
}

bool PDKnob::onMotion(const MotionEvent &ev)
{
    if (isTyping)
        return false;

    if (circularDrag && isActive)
    {
        const Point<int> screen = getScreenPos();
        const double x = ev.pos.getX() - screen.getX();
        const double y = ev.pos.getY() - screen.getY();
        const float cx = getWidth() * 0.5f;
        const float cy = getHeight() * 0.5f;
        const float dx = x - cx;
        const float dy = y - cy;
        const float startAngle = arcBegin - NVG_PI * 0.5f;
        const float endAngle = arcEnd - NVG_PI * 0.5f;
        float angle = std::atan2(dy, dx);
        while (angle < startAngle)
            angle += NVG_PI * 2.0f;
        const float normalized = clamp((angle - startAngle) / (endAngle - startAngle), 0.0f, 1.0f);
        const float range = getMax() - getMin();
        setValue(getMin() + normalized * range, true);
        return true;
    }

    return PDKnobEventHandler::motionEvent(ev);
}

bool PDKnob::onKeyboard(const KeyboardEvent &ev)
{
    if (!isTyping)
        return false;

    if (!ev.press)
        return false;

    if (ev.key == 13 || ev.key == 36) // Enter/Return
    {
        if (!typingText.empty())
        {
            float typedValue = 0.0f;
            if (std::sscanf(typingText.c_str(), "%f", &typedValue) == 1)
            {
                setValue(typedValue, true);
            }
        }
        isTyping = false;
        typingText.clear();
        repaint();
        return true;
    }

    if (ev.key == 27) // Escape
    {
        isTyping = false;
        typingText.clear();
        repaint();
        return true;
    }

    if (ev.key == 8 || ev.key == 127) // Backspace / Delete
    {
        if (!typingText.empty())
        {
            typingText.pop_back();
            repaint();
        }
        return true;
    }

    if (ev.key == 45 || ev.key == 46) // Minus or Period
    {
        // Allow minus only at the start, period only once
        char ch = (char)ev.key;
        if (ch == '-' && typingText.empty())
        {
            typingText += ch;
            repaint();
        }
        else if (ch == '.' && typingText.find('.') == std::string::npos)
        {
            typingText += ch;
            repaint();
        }
        return true;
    }

    // Numeric digits (0-9)
    if (ev.key >= '0' && ev.key <= '9')
    {
        typingText += (char)ev.key;
        repaint();
        return true;
    }

    return true; // Consume all other keys while typing
}

void PDKnob::setColors(NVGcolor bgColor, NVGcolor fgColor, NVGcolor arcColor) {
    this->bgColor = bgColor;
    this->fgColor = fgColor;
    this->arcColor = arcColor;
}

void PDKnob::setLabelStyle(int x, int y, int size)
{
    const float scaleFactor = getTopLevelWidget()->getScaleFactor();

    this->label = new PDLabel(this);
    this->label->setText(" ");
    this->label->setColors(Colors::cnvTextColor);
    this->label->setAbsolutePos((x - 2 * scaleFactor), (y - 4 * scaleFactor));
    this->label->setSize(size, size);
}

void PDKnob::setSteps(int steps) {
    this->steps = steps;
    this->setStep(steps);
}

void PDKnob::setAngular(int angRange, int angOffset) {
    angOffset -= angRange / 2;
    this->arcBegin = degreesToRadians<float>(angOffset) + NVG_PI * 2.0;
    this->arcEnd = degreesToRadians<float>(angRange + angOffset) + NVG_PI * 2.0;
    this->setArc(arcBegin, arcEnd);
}

void PDKnob::setArcStart(float arcStart) {
    this->arcStart = arcStart;
}

void PDKnob::setCircularDrag(bool circularDrag) {
    this->circularDrag = circularDrag;
}

void PDKnob::setDrawSquare(bool drawSquare) {
    this->drawSquare = drawSquare;
}

void PDKnob::setShowTicks(bool showTicks) {
    this->showTicks = showTicks;
}

void PDKnob::setShowArc(bool showArc) {
    this->showArc = showArc;
}

void PDKnob::setShowLabel(LabelShow showLabel) {
    this->showLabel = showLabel;
}

void PDKnob::setImageData(const unsigned char* data, uint32_t width, uint32_t height)
{
    NVGcontext* nvg = getContext();
    if (nvg == nullptr)
        return;

    if (imageHandle >= 0)
    {
        nvgDeleteImage(nvg, imageHandle);
        imageHandle = -1;
    }

    if (data != nullptr && width > 0 && height > 0)
    {
        imageHandle = nvgCreateImageRGBA(nvg, width, height, 0, data);
        imgWidth = width;
        imgHeight = height;
    }
    repaint();
}

void PDKnob::setImageFromMemory(const unsigned char* fileData, uint32_t fileSize)
{
    NVGcontext* nvg = getContext();
    if (nvg == nullptr)
        return;

    if (imageHandle >= 0)
    {
        nvgDeleteImage(nvg, imageHandle);
        imageHandle = -1;
    }

    if (fileData != nullptr && fileSize > 0)
    {
        imageHandle = nvgCreateImageMem(nvg, 0, fileData, fileSize);
        if (imageHandle >= 0)
            nvgImageSize(nvg, imageHandle, (int*)&imgWidth, (int*)&imgHeight);
    }
    repaint();
}


END_NAMESPACE_DISTRHO
