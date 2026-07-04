/*
 * Copyright (C) 2026 Wasted Audio
 * SPDX-License-Identifier: ISC
*/

#include "nanovg.h"
#include <cctype>
#include <cmath>

#include "Fonts/InterRegular.hpp"
#include "Common.hpp"
#include "DragNum.hpp"

START_NAMESPACE_DISTRHO

PDDragNum::PDDragNum(NanoSubWidget *parent, PDDragNumEventHandler::Callback *const cb)
    : PDWidget(parent),
      PDDragNumEventHandler(this)
{
    PDDragNumEventHandler::setCallback(cb);

    using namespace InterRegular;
    NanoVG::FontId interId = createFontFromMemory("inter", (const uchar *)InterRegularData, InterRegularDataSize, 0);
    fFontId = interId;
}

std::string PDDragNum::formatNumber(float value) {
    // %.6g strips trailing zeros and omits decimal point for integers
    char buffer[64];
    std::snprintf(buffer, sizeof(buffer), "%.6g", (double)value);
    return buffer;
}

int PDDragNum::findHoveredDigit(float mouseX)
{
    if (currentValue.empty())
        return -1;

    NVGcontext* nvg = getContext();
    nvgFontFaceId(nvg, fFontId);
    nvgFontSize(nvg, getHeight() * 0.862f);
    nvgTextLetterSpacing(nvg, 0.15f);
    nvgTextAlign(nvg, NVG_ALIGN_MIDDLE | NVG_ALIGN_LEFT);

    float textX = border.left;
    float cursorX = textX;

    for (size_t i = 0; i < currentValue.size(); ++i) {
        char str[2] = { currentValue[i], '\0' };
        float advance = nvgText(nvg, 0, 0, str, nullptr);

        if (mouseX >= cursorX && mouseX < cursorX + advance) {
            return (int)i;
        }
        cursorX += advance;
    }

    return -1;
}

void PDDragNum::onNanoDisplay()
{
    scaleFactor_ = getTopLevelWidget()->getScaleFactor();
    const DGL::Rectangle<float> b(0.0f, 0.0f, getWidth(), getHeight());

    NVGcontext* nvg = getContext();

    currentValue = formatNumber(getValue());

    nvgIntersectScissor(nvg, 0.5f, 0.5f, getWidth() - 1 * scaleFactor_, getHeight() - 1 * scaleFactor_);

    nvgFontFaceId(nvg, fFontId);
    nvgFontSize(nvg, getHeight() * 0.862f);
    nvgTextLetterSpacing(nvg, 0.15f);
    nvgTextAlign(nvg, NVG_ALIGN_MIDDLE | NVG_ALIGN_LEFT);

    auto const textArea = subtractBorder(b, border);
    float textY = textArea.getY() + textArea.getHeight() / 2.0f + 1.5f * scaleFactor_;

    // Calculate hovered digit bounding box before drawing highlight
    if (hoveredDecimal >= 0 && hoveredDecimal < (int)currentValue.size()) {
        float charX = textArea.getX();
        for (int i = 0; i < hoveredDecimal; ++i) {
            char str[2] = { currentValue[i], '\0' };
            charX += nvgText(nvg, 0, 0, str, nullptr);
        }
        char str[2] = { currentValue[hoveredDecimal], '\0' };
        float charW = nvgText(nvg, 0, 0, str, nullptr);
        hoveredDecimalPosition = DGL::Rectangle<float>(
            charX, textArea.getY(), charW, textArea.getHeight());

        const float alpha = isDragging() ? 0.5f : 0.3f;
        auto const highlightColour = nvgRGBA(outlineColor.r, outlineColor.g, outlineColor.b, alpha * 255);
        drawRoundedRect(nvg, hoveredDecimalPosition.getX(), hoveredDecimalPosition.getY() - 1,
                        hoveredDecimalPosition.getWidth(), hoveredDecimalPosition.getHeight(),
                        highlightColour, highlightColour, 2.5f);
    }

    nvgFillColor(nvg, textColor);
    auto const listText = currentValue;
    nvgText(nvg, textArea.getX(), textY, listText.c_str(), nullptr);
}

bool PDDragNum::onMouse(const MouseEvent &ev)
{
    if (ev.press && ev.button == 1) {
        // Detect which digit the mouse is over at click time
        const Point<int> screen = getScreenPos();
        float localX = (float)(ev.pos.getX() - screen.getX());
        int digitIdx = findHoveredDigit(localX);
        if (digitIdx >= 0) {
            hoveredDecimal = digitIdx;
        }
    }
    return PDDragNumEventHandler::mouseEvent(ev);
}

bool PDDragNum::onMotion(const MotionEvent &ev)
{
    // Update which digit is hovered during motion (even when not dragging)
    const Point<int> screen = getScreenPos();
    float localX = (float)(ev.pos.getX() - screen.getX());
    float localY = (float)(ev.pos.getY() - screen.getY());
    const DGL::Rectangle<float> bounds(0.0f, 0.0f, getWidth(), getHeight());

    if (bounds.contains(localX, localY)) {
        int digitIdx = findHoveredDigit(localX);
        if (digitIdx != hoveredDecimal) {
            hoveredDecimal = digitIdx;
            repaint();
        }
    } else if (hoveredDecimal >= 0) {
        hoveredDecimal = -1;
        repaint();
    }

    return PDDragNumEventHandler::motionEvent(ev);
}

bool PDDragNum::onKeyboard(const KeyboardEvent &ev)
{
    return PDDragNumEventHandler::keyboardEvent(ev);
}

void PDDragNum::setColors(NVGcolor outlineColor, NVGcolor textColor) {
    this->outlineColor = outlineColor;
    this->textColor = textColor;
}

void PDDragNum::setLogMode(bool log) {
    PDDragNumEventHandler::setLogMode(log);
}

END_NAMESPACE_DISTRHO
