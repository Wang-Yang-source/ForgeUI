#include "Exporter.h"

namespace forgeui_studio {

QString exportForgeUiSource(const SceneModel& scene) {
    QString output;
    output += "#include <forgeui/ForgeUI.h>\n\n";
    output += QString("forgeui::Rgb565Canvas<%1, %2> canvas;\n\n")
                  .arg(scene.canvasSize.width()).arg(scene.canvasSize.height());
    output += "void renderForgeUiScene() {\n";
    output += "    canvas.clear(forgeui::Color::black());\n";
    for (const SceneNode& node : scene.nodes) {
        const int x0 = static_cast<int>(node.rect.x());
        const int y0 = static_cast<int>(node.rect.y());
        const int x1 = x0 + static_cast<int>(node.rect.width()) - 1;
        const int y1 = y0 + static_cast<int>(node.rect.height()) - 1;
        output += QString("    // %1 (%2)\n").arg(node.id, node.type);
        output += QString("    canvas.box(%1, %2, %3, %4, forgeui::Color{%5, %6, %7});\n")
                      .arg(x0).arg(y0).arg(x1).arg(y1)
                      .arg(node.color.red()).arg(node.color.green()).arg(node.color.blue());
        if (node.type == "label") output += QString("    // label text: %1\n").arg(node.text);
    }
    output += "}\n";
    return output;
}

} // namespace forgeui_studio
