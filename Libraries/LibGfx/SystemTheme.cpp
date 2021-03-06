/*
 * Copyright (c) 2018-2020, Andreas Kling <kling@serenityos.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <AK/SharedBuffer.h>
#include <LibCore/ConfigFile.h>
#include <LibGfx/SystemTheme.h>

namespace Gfx {

static SystemTheme dummy_theme;
static const SystemTheme* theme_page = &dummy_theme;
static RefPtr<SharedBuffer> theme_buffer;

const SystemTheme& current_system_theme()
{
    ASSERT(theme_page);
    return *theme_page;
}

int current_system_theme_buffer_id()
{
    ASSERT(theme_buffer);
    return theme_buffer->shbuf_id();
}

void set_system_theme(SharedBuffer& buffer)
{
    theme_buffer = buffer;
    theme_page = (SystemTheme*)theme_buffer->data();
}

RefPtr<SharedBuffer> load_system_theme(const String& path)
{
    auto file = Core::ConfigFile::open(path);
    auto buffer = SharedBuffer::create_with_size(sizeof(SystemTheme));

    auto* data = (SystemTheme*)buffer->data();

    auto get_color = [&](auto& name) {
        auto color_string = file->read_entry("Colors", name);
        auto color = Color::from_string(color_string);
        if (!color.has_value())
            return Color(Color::Black);
        return color.value();
    };

    auto get_metric = [&](auto& name, auto role) {
        int metric = file->read_num_entry("Metrics", name, -1);
        if (metric == -1) {
            switch (role) {
            case (int)MetricRole::TitleHeight:
                return 19;
            case (int)MetricRole::TitleButtonHeight:
                return 15;
            case (int)MetricRole::TitleButtonWidth:
                return 15;
            default:
                dbg() << "Metric " << name << " has no fallback value!";
                return 16;
            }
        }
        return metric;
    };

    auto get_path = [&](auto& name, auto role) {
        auto path = file->read_entry("Paths", name);
        if (path.is_empty()) {
            switch (role) {
            case (int)PathRole::TitleButtonIcons:
                return "/res/icons/16x16/";
            default:
                return "/res/";
            }
        }
        return &path[0];
    };

#define DO_COLOR(x) \
    data->color[(int)ColorRole::x] = get_color(#x)

    DO_COLOR(DesktopBackground);
    DO_COLOR(ThreedHighlight);
    DO_COLOR(ThreedShadow1);
    DO_COLOR(ThreedShadow2);
    DO_COLOR(HoverHighlight);
    DO_COLOR(Selection);
    DO_COLOR(SelectionText);
    DO_COLOR(InactiveSelection);
    DO_COLOR(InactiveSelectionText);
    DO_COLOR(Window);
    DO_COLOR(WindowText);
    DO_COLOR(Base);
    DO_COLOR(BaseText);
    DO_COLOR(Button);
    DO_COLOR(ButtonText);
    DO_COLOR(DesktopBackground);
    DO_COLOR(ActiveWindowBorder1);
    DO_COLOR(ActiveWindowBorder2);
    DO_COLOR(ActiveWindowTitle);
    DO_COLOR(ActiveWindowTitleShadow);
    DO_COLOR(ActiveWindowTitleStripes);
    DO_COLOR(InactiveWindowBorder1);
    DO_COLOR(InactiveWindowBorder2);
    DO_COLOR(InactiveWindowTitle);
    DO_COLOR(InactiveWindowTitleShadow);
    DO_COLOR(InactiveWindowTitleStripes);
    DO_COLOR(MovingWindowBorder1);
    DO_COLOR(MovingWindowBorder2);
    DO_COLOR(MovingWindowTitle);
    DO_COLOR(MovingWindowTitleShadow);
    DO_COLOR(MovingWindowTitleStripes);
    DO_COLOR(HighlightWindowBorder1);
    DO_COLOR(HighlightWindowBorder2);
    DO_COLOR(HighlightWindowTitle);
    DO_COLOR(HighlightWindowTitleShadow);
    DO_COLOR(HighlightWindowTitleStripes);
    DO_COLOR(MenuStripe);
    DO_COLOR(MenuBase);
    DO_COLOR(MenuBaseText);
    DO_COLOR(MenuSelection);
    DO_COLOR(MenuSelectionText);
    DO_COLOR(RubberBandFill);
    DO_COLOR(RubberBandBorder);
    DO_COLOR(Link);
    DO_COLOR(ActiveLink);
    DO_COLOR(VisitedLink);
    DO_COLOR(Ruler);
    DO_COLOR(RulerBorder);
    DO_COLOR(RulerActiveText);
    DO_COLOR(RulerInactiveText);
    DO_COLOR(TextCursor);
    DO_COLOR(FocusOutline);
    DO_COLOR(SyntaxComment);
    DO_COLOR(SyntaxNumber);
    DO_COLOR(SyntaxString);
    DO_COLOR(SyntaxType);
    DO_COLOR(SyntaxPunctuation);
    DO_COLOR(SyntaxOperator);
    DO_COLOR(SyntaxKeyword);
    DO_COLOR(SyntaxControlKeyword);
    DO_COLOR(SyntaxIdentifier);
    DO_COLOR(SyntaxPreprocessorStatement);
    DO_COLOR(SyntaxPreprocessorValue);

#define DO_METRIC(x) \
    data->metric[(int)MetricRole::x] = get_metric(#x, (int)MetricRole::x)

    DO_METRIC(TitleHeight);
    DO_METRIC(TitleButtonWidth);
    DO_METRIC(TitleButtonHeight);

#define DO_PATH(x) \
    data->path[(int)PathRole::x] = get_path(#x, (int)PathRole::x)

    DO_PATH(TitleButtonIcons);

    buffer->seal();
    buffer->share_globally();

    return buffer;
}

}
