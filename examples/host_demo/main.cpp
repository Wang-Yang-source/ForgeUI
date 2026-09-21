#include <cassert>
#include <iostream>
#include <forgeui/ForgeUI.h>

template <typename CanvasT>
class Marker : public forgeui::Component<CanvasT> {
public:
    explicit Marker(int& updates) : updates_(updates) {}
    void update(uint32_t) override { ++updates_; }
    void draw(CanvasT& canvas) const override { canvas.pixel(1, 1); }
    bool handle(const forgeui::InputEvent& event) override {
        return event.type == forgeui::InputType::Click;
    }
private:
    int& updates_;
};

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

    forgeui::Tween reverse;
    reverse.start(0, 100, forgeui::Easing::Linear, 255, 0);
    assert(reverse.value(50) < 255 && reverse.value(50) > 0);

    forgeui::MonoCanvas<8, 8> mono;
    mono.clear();
    mono.box(0, 0, 7, 7);
    const auto& monoView = mono;
    assert(monoView.pixel(0, 0));
    assert(!monoView.pixel(3, 3));

    forgeui::Rgb565Canvas<4, 4> colorCanvas;
    colorCanvas.clear(forgeui::Color{255, 0, 0});
    assert(colorCanvas.data()[0] == 0xf800);
    colorCanvas.pixel(2, 2, forgeui::Color{0, 255, 0});
    assert(colorCanvas.data()[10] == 0x07e0);

    int updates = 0;
    Marker<forgeui::MonoCanvas<8, 8>> marker(updates);
    forgeui::StaticContainer<forgeui::MonoCanvas<8, 8>, 2> container;
    assert(container.add(marker));
    container.update(42);
    assert(updates == 1);
    assert(container.handle({forgeui::InputType::Click, 0, 42}));

    forgeui::Rgb888Canvas<2, 2> rgb;
    rgb.clear(forgeui::Color{1, 2, 3});
    assert(rgb.data()[0] == 1 && rgb.data()[1] == 2 && rgb.data()[2] == 3);
    const uint16_t uneven[] = {90, 40, 180, 70};
    forgeui::Typewriter<8> typewriter;
    assert(typewriter.configure(uneven, 4, 100));
    assert(typewriter.visible(190) == 1 && typewriter.visible(230) == 2);

    int32_t logoY = 0;
    int32_t typedChars = 0;
    forgeui::Timeline<4> intro;
    assert(intro.add(logoY, -12, 18, 0, 800));
    assert(intro.add(typedChars, 0, 7, 250, 1200, forgeui::Easing::EaseOut));
    intro.start(1000);
    intro.update(1250);
    assert(logoY > -12 && typedChars == 0);
    intro.update(2200);
    assert(logoY == 18 && typedChars > 0);

    std::cout << "ForgeUI host demo passed\n";
}
