from __future__ import annotations

from datetime import datetime
from pathlib import Path

from reportlab.graphics.shapes import Drawing, Line, Polygon, Rect, String
from reportlab.lib import colors
from reportlab.lib.pagesizes import A4
from reportlab.lib.styles import ParagraphStyle, getSampleStyleSheet
from reportlab.lib.units import cm
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont
from reportlab.platypus import (
    PageBreak,
    Paragraph,
    Preformatted,
    SimpleDocTemplate,
    Spacer,
    Table,
    TableStyle,
)

ROOT = Path(__file__).resolve().parents[1]
OUT_PDF = ROOT / "docs" / "GitHub_STM32_Deploy_Guide.pdf"


def pick_font() -> str:
    candidates = [
        Path(r"C:\Windows\Fonts\NotoSansSC-Regular.ttf"),
        Path(r"C:\Windows\Fonts\msyh.ttc"),
        Path(r"C:\Windows\Fonts\simhei.ttf"),
    ]
    for font_path in candidates:
        if font_path.exists():
            pdfmetrics.registerFont(TTFont("ZH", str(font_path)))
            return "ZH"
    raise FileNotFoundError("未找到可用中文字体。请确认系统已安装 Noto Sans SC / 微软雅黑 / 黑体。")


def add_arrow(d: Drawing, x1: float, y1: float, x2: float, y2: float) -> None:
    color = colors.HexColor("#2F5597")
    d.add(Line(x1, y1, x2, y2, strokeColor=color, strokeWidth=1.6))
    d.add(Polygon([x2, y2, x2 - 7, y2 + 3.5, x2 - 7, y2 - 3.5], fillColor=color, strokeColor=color))


def figure_flow(font_name: str) -> Drawing:
    d = Drawing(510, 130)
    box_fill = colors.HexColor("#EAF1FB")
    box_line = colors.HexColor("#4A76B8")

    nodes = [
        (8, 48, 92, 42, "Step 1\n获取仓库"),
        (112, 48, 92, 42, "Step 2\n安装工具链"),
        (216, 48, 92, 42, "Step 3\n连接硬件"),
        (320, 48, 92, 42, "Step 4\n编译与烧录"),
        (424, 48, 78, 42, "Step 5\n联调验收"),
    ]

    for x, y, w, h, label in nodes:
        d.add(Rect(x, y, w, h, rx=7, ry=7, fillColor=box_fill, strokeColor=box_line, strokeWidth=1.2))
        lines = label.split("\n")
        for i, line in enumerate(lines):
            d.add(
                String(
                    x + w / 2,
                    y + h / 2 + 6 - i * 12,
                    line,
                    fontName=font_name,
                    fontSize=9,
                    textAnchor="middle",
                    fillColor=colors.HexColor("#1E2E4F"),
                )
            )

    for i in range(len(nodes) - 1):
        x, y, w, h, _ = nodes[i]
        nx, ny, _, nh, _ = nodes[i + 1]
        add_arrow(d, x + w + 4, y + h / 2, nx - 5, ny + nh / 2)

    d.add(String(8, 108, "图 1  部署流程总览", fontName=font_name, fontSize=9, fillColor=colors.HexColor("#1E2E4F")))
    return d


def figure_topology(font_name: str) -> Drawing:
    d = Drawing(510, 200)

    # MCU
    d.add(Rect(198, 78, 114, 82, rx=10, ry=10, fillColor=colors.HexColor("#FFF4D6"), strokeColor=colors.HexColor("#D29F2E"), strokeWidth=1.4))
    d.add(String(255, 118, "STM32G030", textAnchor="middle", fontName=font_name, fontSize=10))
    d.add(String(255, 102, "I2C1 / ADC / UART", textAnchor="middle", fontName=font_name, fontSize=8))

    # AHT20
    d.add(Rect(44, 124, 120, 46, rx=8, ry=8, fillColor=colors.HexColor("#E8F7EE"), strokeColor=colors.HexColor("#3D8A5B"), strokeWidth=1.2))
    d.add(String(104, 148, "AHT20 温湿度传感器", textAnchor="middle", fontName=font_name, fontSize=9))

    # OLED
    d.add(Rect(44, 52, 120, 46, rx=8, ry=8, fillColor=colors.HexColor("#E8F7EE"), strokeColor=colors.HexColor("#3D8A5B"), strokeWidth=1.2))
    d.add(String(104, 76, "SSD1306 OLED", textAnchor="middle", fontName=font_name, fontSize=9))

    # NTC
    d.add(Rect(346, 120, 132, 50, rx=8, ry=8, fillColor=colors.HexColor("#FEECEC"), strokeColor=colors.HexColor("#B85450"), strokeWidth=1.2))
    d.add(String(412, 146, "NTC + 分压电阻", textAnchor="middle", fontName=font_name, fontSize=9))

    # ST-LINK
    d.add(Rect(346, 46, 132, 50, rx=8, ry=8, fillColor=colors.HexColor("#EDF1FF"), strokeColor=colors.HexColor("#5A6FD1"), strokeWidth=1.2))
    d.add(String(412, 72, "ST-LINK / 串口调试", textAnchor="middle", fontName=font_name, fontSize=9))

    line_color = colors.HexColor("#2F5597")
    d.add(Line(164, 146, 198, 146, strokeColor=line_color, strokeWidth=1.4))
    d.add(String(168, 150, "I2C1", fontName=font_name, fontSize=8))

    d.add(Line(164, 74, 198, 94, strokeColor=line_color, strokeWidth=1.4))
    d.add(String(166, 82, "I2C1", fontName=font_name, fontSize=8))

    d.add(Line(346, 146, 312, 130, strokeColor=line_color, strokeWidth=1.4))
    d.add(String(318, 138, "ADC", fontName=font_name, fontSize=8))

    d.add(Line(346, 70, 312, 92, strokeColor=line_color, strokeWidth=1.4))
    d.add(String(316, 84, "SWD/UART", fontName=font_name, fontSize=8))

    d.add(String(8, 180, "图 2  硬件连接拓扑（示意）", fontName=font_name, fontSize=9, fillColor=colors.HexColor("#1E2E4F")))
    return d


def footer(canvas, doc, font_name: str) -> None:
    canvas.saveState()
    canvas.setFont(font_name, 8)
    canvas.setFillColor(colors.HexColor("#667085"))
    canvas.drawString(2 * cm, 1.0 * cm, "GitHub 仓库下载与 STM32 本地部署教程")
    canvas.drawRightString(A4[0] - 2 * cm, 1.0 * cm, f"第 {canvas.getPageNumber()} 页")
    canvas.restoreState()


def build_styles(font_name: str) -> dict[str, ParagraphStyle]:
    base = getSampleStyleSheet()
    return {
        "title": ParagraphStyle(
            "TitleZH",
            parent=base["Title"],
            fontName=font_name,
            fontSize=24,
            leading=30,
            textColor=colors.HexColor("#12355B"),
            alignment=1,
            spaceAfter=12,
        ),
        "subtitle": ParagraphStyle(
            "SubtitleZH",
            parent=base["Normal"],
            fontName=font_name,
            fontSize=12,
            leading=18,
            textColor=colors.HexColor("#344054"),
            alignment=1,
        ),
        "h1": ParagraphStyle(
            "H1ZH",
            parent=base["Heading1"],
            fontName=font_name,
            fontSize=15,
            leading=22,
            textColor=colors.HexColor("#0B3A6E"),
            spaceBefore=10,
            spaceAfter=6,
        ),
        "body": ParagraphStyle(
            "BodyZH",
            parent=base["BodyText"],
            fontName=font_name,
            fontSize=10.5,
            leading=18,
            textColor=colors.HexColor("#1D2939"),
            spaceAfter=6,
        ),
        "note": ParagraphStyle(
            "NoteZH",
            parent=base["BodyText"],
            fontName=font_name,
            fontSize=9.5,
            leading=15,
            textColor=colors.HexColor("#475467"),
            leftIndent=10,
            borderPadding=6,
            backColor=colors.HexColor("#F8FAFC"),
            spaceBefore=6,
            spaceAfter=8,
        ),
        "code": ParagraphStyle(
            "CodeZH",
            parent=base["Code"],
            fontName="Courier",
            fontSize=8.8,
            leading=12,
            backColor=colors.HexColor("#F4F6F8"),
            borderPadding=6,
            leftIndent=2,
            rightIndent=2,
        ),
    }


def build_story(font_name: str, styles: dict[str, ParagraphStyle]) -> list:
    story = []

    story.append(Spacer(1, 3.2 * cm))
    story.append(Paragraph("GitHub 仓库下载与 STM32 本地部署教程", styles["title"]))
    story.append(Paragraph("面向硬件培训同学的标准化上手文档", styles["subtitle"]))
    story.append(Spacer(1, 0.4 * cm))
    story.append(Paragraph("项目：AHT20 + SSD1306 + NTC 多传感器数据采集系统", styles["subtitle"]))
    story.append(Paragraph(f"版本日期：{datetime.now().strftime('%Y-%m-%d')}", styles["subtitle"]))
    story.append(Spacer(1, 1.2 * cm))
    story.append(Paragraph("培训目标", styles["h1"]))
    story.append(Paragraph("通过本教程，同学可以在自己的电脑上完成仓库克隆、工程编译、固件烧录、串口验证与基础故障定位，形成可复现实验流程。", styles["body"]))
    story.append(Paragraph("产出标准：设备可稳定输出温湿度与温度融合结果，OLED 与串口日志均可观察到有效数据。", styles["note"]))

    story.append(PageBreak())

    story.append(Paragraph("1. 部署流程总览", styles["h1"]))
    story.append(Paragraph("推荐按以下 5 个阶段执行，每一步都给出可量化的验收点。", styles["body"]))
    story.append(figure_flow(font_name))
    story.append(Spacer(1, 0.3 * cm))

    checklist_data = [
        ["阶段", "关键动作", "验收标准"],
        ["Step 1", "从 GitHub 获取源码", "本地出现 `tutorial` 目录且可见 `MDK-ARM/Desktop.uvprojx`"],
        ["Step 2", "安装 Keil / 驱动 / 串口工具", "工具可正常启动，ST-LINK 在设备管理器可见"],
        ["Step 3", "完成传感器与 MCU 接线", "供电正常，无过热与短路"],
        ["Step 4", "编译并下载固件", "Keil 编译零错误，Flash Download 成功"],
        ["Step 5", "串口与 OLED 联调", "串口连续打印，OLED 显示温湿度数据"],
    ]

    checklist = Table(checklist_data, colWidths=[2.0 * cm, 5.3 * cm, 8.0 * cm])
    checklist.setStyle(
        TableStyle(
            [
                ("FONTNAME", (0, 0), (-1, -1), font_name),
                ("FONTSIZE", (0, 0), (-1, 0), 10),
                ("FONTSIZE", (0, 1), (-1, -1), 9),
                ("BACKGROUND", (0, 0), (-1, 0), colors.HexColor("#DCE8F8")),
                ("TEXTCOLOR", (0, 0), (-1, 0), colors.HexColor("#102A43")),
                ("GRID", (0, 0), (-1, -1), 0.6, colors.HexColor("#9DB5D5")),
                ("VALIGN", (0, 0), (-1, -1), "MIDDLE"),
                ("ROWBACKGROUNDS", (0, 1), (-1, -1), [colors.white, colors.HexColor("#F8FBFF")]),
            ]
        )
    )
    story.append(checklist)

    story.append(Spacer(1, 0.4 * cm))
    story.append(Paragraph("2. 从 GitHub 下载仓库", styles["h1"]))
    story.append(Paragraph("2.1 HTTPS 方式（通用，首次推荐）", styles["body"]))
    story.append(
        Preformatted(
            "git clone https://github.com/sea90d/tutorial.git\ncd tutorial",
            styles["code"],
        )
    )
    story.append(Paragraph("2.2 SSH 方式（已配置 key 的同学）", styles["body"]))
    story.append(
        Preformatted(
            "git clone git@github.com:sea90d/tutorial.git\ncd tutorial",
            styles["code"],
        )
    )
    story.append(Paragraph("建议使用 `git pull` 同步更新，避免直接下载 ZIP 后丢失版本管理能力。", styles["note"]))

    story.append(PageBreak())

    story.append(Paragraph("3. 本地环境与硬件连接", styles["h1"]))
    story.append(Paragraph("3.1 软件环境", styles["body"]))
    env_table = Table(
        [
            ["组件", "建议版本", "用途"],
            ["Keil MDK", "MDK5", "打开与编译 `MDK-ARM/Desktop.uvprojx`"],
            ["ST-LINK 驱动", "最新版", "下载固件与在线调试"],
            ["串口终端", "任意", "观察 UART 日志（115200, 8N1）"],
        ],
        colWidths=[3.0 * cm, 3.0 * cm, 9.3 * cm],
    )
    env_table.setStyle(
        TableStyle(
            [
                ("FONTNAME", (0, 0), (-1, -1), font_name),
                ("BACKGROUND", (0, 0), (-1, 0), colors.HexColor("#E7F4EE")),
                ("GRID", (0, 0), (-1, -1), 0.5, colors.HexColor("#A3C9B4")),
                ("FONTSIZE", (0, 0), (-1, -1), 9.4),
            ]
        )
    )
    story.append(env_table)
    story.append(Spacer(1, 0.25 * cm))

    story.append(Paragraph("3.2 硬件连接建议", styles["body"]))
    story.append(figure_topology(font_name))
    story.append(Paragraph("接口说明：I2C1(PB6/PB7) 连接 AHT20 与 SSD1306，ADC_IN0(PA0) 读取 NTC 分压，USART1(PA9/PA10) 输出调试日志。", styles["note"]))

    story.append(Paragraph("4. 编译、烧录与运行验证", styles["h1"]))
    story.append(Paragraph("在 Keil 打开工程后，建议先执行 Rebuild，再点击 Download。", styles["body"]))
    story.append(
        Preformatted(
            "# 典型启动日志（示例）\r\n"
            "System init done\r\n"
            "I2C scan: found device 0x38 (AHT20)\r\n"
            "I2C scan: found device 0x3C (SSD1306)\r\n"
            "AHT20: T=25.3C RH=48.6%\r\n"
            "NTC: T=25.8C\r\n"
            "T_avg=25.6C\r\n",
            styles["code"],
        )
    )
    story.append(Paragraph("若串口无输出，优先检查：串口波特率、PA9/PA10 接线、地线共地、`printf` 重定向。", styles["note"]))

    story.append(PageBreak())

    story.append(Paragraph("5. 常见问题与定位策略", styles["h1"]))
    qa_table = Table(
        [
            ["问题", "现象", "快速排查"],
            ["无法拉取仓库", "`git push/pull` 连接失败", "检查代理、DNS、SSH key 或切换至 `ssh.github.com:443`"],
            ["AHT20 无数据", "始终读到 0 或 CRC 失败", "确认地址 0x38、上拉电阻、I2C 时序与供电"],
            ["OLED 不显示", "屏幕常亮黑屏", "确认地址 0x3C 与初始化顺序，核对 VCC/GND"],
            ["NTC 波动大", "温度值抖动明显", "增加采样平均与 ADC 滤波，检查分压电阻精度"],
        ],
        colWidths=[3.2 * cm, 5.0 * cm, 7.1 * cm],
    )
    qa_table.setStyle(
        TableStyle(
            [
                ("FONTNAME", (0, 0), (-1, -1), font_name),
                ("BACKGROUND", (0, 0), (-1, 0), colors.HexColor("#FDECC8")),
                ("GRID", (0, 0), (-1, -1), 0.5, colors.HexColor("#E3B86C")),
                ("FONTSIZE", (0, 0), (-1, -1), 9.2),
            ]
        )
    )
    story.append(qa_table)

    story.append(Spacer(1, 0.35 * cm))
    story.append(Paragraph("6. 培训验收建议（可量化）", styles["h1"]))
    story.append(Paragraph("1) 代码可独立拉取与编译；2) 板卡可稳定运行 30 分钟；3) 串口每秒输出有效数据；4) OLED 显示与串口一致；5) 能解释关键外设映射与数据链路。", styles["body"]))

    story.append(Spacer(1, 0.8 * cm))
    story.append(Paragraph("附：一键重新生成 PDF", styles["h1"]))
    story.append(
        Preformatted(
            "python docs/generate_deploy_guide_pdf.py",
            styles["code"],
        )
    )

    return story


def main() -> None:
    font_name = pick_font()
    styles = build_styles(font_name)

    doc = SimpleDocTemplate(
        str(OUT_PDF),
        pagesize=A4,
        leftMargin=2.0 * cm,
        rightMargin=2.0 * cm,
        topMargin=2.0 * cm,
        bottomMargin=1.8 * cm,
        title="GitHub仓库下载与STM32本地部署教程",
        author="tutorial training team",
        subject="STM32 deployment guide",
    )

    story = build_story(font_name, styles)
    doc.build(story, onFirstPage=lambda c, d: footer(c, d, font_name), onLaterPages=lambda c, d: footer(c, d, font_name))

    print(f"PDF generated: {OUT_PDF}")


if __name__ == "__main__":
    main()
