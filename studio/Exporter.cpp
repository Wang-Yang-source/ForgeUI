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

QString exportLvglSource(const SceneModel& scene) {
    QString output;
    output += "#include <lvgl.h>\n\n";
    output += QString("lv_obj_t* forgeui_build_screen(lv_obj_t* parent) {\n    // %1x%2 scene\n")
                  .arg(scene.canvasSize.width()).arg(scene.canvasSize.height());
    output += "    lv_obj_t* root = lv_obj_create(parent);\n";
    output += QString("    lv_obj_set_size(root, %1, %2);\n")
                  .arg(scene.canvasSize.width()).arg(scene.canvasSize.height());
    output += "    lv_obj_clear_flag(root, LV_OBJ_FLAG_SCROLLABLE);\n";
    for (int i = 0; i < scene.nodes.size(); ++i) {
        const SceneNode& node = scene.nodes[i];
        const int x = static_cast<int>(node.rect.x());
        const int y = static_cast<int>(node.rect.y());
        const int width = static_cast<int>(node.rect.width());
        const int height = static_cast<int>(node.rect.height());
        const QString color = node.color.name(QColor::HexRgb);
        if (node.type == "label") {
            output += QString("    lv_obj_t* label_%1 = lv_label_create(root);\n").arg(i);
            output += QString("    lv_label_set_text(label_%1, \"%2\");\n").arg(i).arg(node.text);
            output += QString("    lv_obj_set_pos(label_%1, %2, %3);\n").arg(i).arg(x).arg(y);
            output += QString("    lv_obj_set_style_text_color(label_%1, lv_color_hex(0x%2), 0);\n")
                          .arg(i).arg(color.mid(1));
        } else {
            output += QString("    lv_obj_t* box_%1 = lv_obj_create(root);\n").arg(i);
            output += QString("    lv_obj_set_size(box_%1, %2, %3);\n").arg(i).arg(width).arg(height);
            output += QString("    lv_obj_set_pos(box_%1, %2, %3);\n").arg(i).arg(x).arg(y);
            output += QString("    lv_obj_set_style_bg_opa(box_%1, LV_OPA_TRANSP, 0);\n").arg(i);
            output += QString("    lv_obj_set_style_border_color(box_%1, lv_color_hex(0x%2), 0);\n")
                          .arg(i).arg(color.mid(1));
        }
    }
    output += "    return root;\n}\n";
    return output;
}

QString exportSlintSource(const SceneModel& scene) {
    QString output;
    output += QString("export component ForgeScene inherits Rectangle {\n    width: %1px;\n    height: %2px;\n    background: #05080c;\n")
                  .arg(scene.canvasSize.width()).arg(scene.canvasSize.height());
    for (int i = 0; i < scene.nodes.size(); ++i) {
        const SceneNode& node = scene.nodes[i];
        const QString color = node.color.name(QColor::HexRgb);
        if (node.type == "label") {
            output += QString("    Text { x: %1px; y: %2px; width: %3px; height: %4px; text: \"%5\"; color: %6; }\n")
                          .arg(node.rect.x()).arg(node.rect.y()).arg(node.rect.width()).arg(node.rect.height())
                          .arg(node.text).arg(color);
        } else {
            output += QString("    Rectangle { x: %1px; y: %2px; width: %3px; height: %4px; border-width: 1px; border-color: %5; background: transparent; }\n")
                          .arg(node.rect.x()).arg(node.rect.y()).arg(node.rect.width()).arg(node.rect.height())
                          .arg(color);
        }
    }
    output += "}\n";
    return output;
}

} // namespace forgeui_studio
