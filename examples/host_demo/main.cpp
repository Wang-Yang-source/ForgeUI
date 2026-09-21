#include <cassert>
#include <iostream>
#include <forgeui/ForgeUI.h>

int main() {
    forgeui::Canvas<16, 8> canvas;
    canvas.clear();
    canvas.box(0, 0, 15, 7, forgeui::Color::white());
    canvas.line(0, 0, 15, 7, forgeui::Color::cyan());
    assert(canvas.data()[0].r == 0 && canvas.data()[0].g == 220);

    forgeui::Tween tween;
    tween.start(100, 1000);
    assert(tween.value(100) == 0);
    assert(tween.value(1100) == 255);
    std::cout << "ForgeUI host demo passed\n";
}
