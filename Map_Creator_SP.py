import sys
from PyQt6.QtWidgets import (QApplication, QMainWindow, QWidget, QVBoxLayout,
                             QHBoxLayout, QPushButton, QLabel,
                             QFileDialog, QMessageBox, QScrollArea, QFrame,
                             QDialog, QSpinBox, QDialogButtonBox, QFormLayout)
from PyQt6.QtCore import Qt, QPoint, QSize
from PyQt6.QtGui import QPainter, QColor, QMouseEvent, QAction, QIcon, QPalette, QFont

# ===== Map Tiles =====
# Dictionary: {Code: (Name, Color, Icon)}
TILES = {
    # Floor and Wall Types
    0x00: ("Floor", QColor(50, 50, 50), "•"),
    0xF6: ("Wall Type 1", QColor(100, 100, 100), "▓"),
    0xF7: ("Wall Left", QColor(120, 120, 120), "◄"),
    0xF8: ("Wall Right", QColor(140, 140, 140), "►"),
    0xF9: ("Wall Medkit", QColor(160, 80, 200), "+"),
    0xFA: ("Wall Charge", QColor(200, 180, 80), "☼"),
    0xFB: ("Level Door", QColor(100, 200, 100), "↕"),
    0xFC: ("Door", QColor(205, 133, 63), "×"),
    0xFD: ("Illuminator", QColor(255, 255, 0), "O"),
    0xFE: ("Monitor", QColor(139, 69, 19), "#"),
    0xF5: ("Exit", QColor(0, 255, 0), "☺"),
    
    # Player
    0x01: ("Player Start", QColor(255, 255, 255), "*"),
    
    # Enemy Types
    0x03: ("Nub", QColor(255, 100, 100), "1"),
    0x04: ("Easy Enemy", QColor(200, 150, 50), "2"),
    0x05: ("Medium Enemy", QColor(255, 140, 0), "3"),
    0x06: ("Hard Enemy", QColor(220, 20, 60), "4"),
    0x07: ("Boss 1", QColor(178, 34, 34), "5"),
    0x08: ("Boss 2", QColor(255, 99, 71), "6"),
    0x09: ("Boss 3", QColor(139, 0, 0), "7"),
    0x0A: ("Big Potato", QColor(210, 105, 30), "8"),
    
    # Weapon Pickups
    0x0D: ("Blaster", QColor(255, 0, 255), "B"),
    0x0E: ("Plasma Cutter", QColor(255, 20, 147), "P"),
    0x0F: ("BFG9000", QColor(148, 0, 211), "F"),
    
    # Item Types
    0x10: ("Book 1", QColor(0, 191, 255), "c"),
    0x11: ("Book 2", QColor(135, 206, 250), "s"),
    0x12: ("Book 3", QColor(173, 216, 230), "i"),
    0x13: ("Sepulki", QColor(255, 215, 0), "§"),
    0x14: ("Medkit", QColor(50, 205, 50), "♥"),
    0x15: ("Ammo", QColor(238, 232, 170), "♦"),
    0x17: ("Metal Box", QColor(128, 128, 128), "m"),
    
    # Special Blocks
    0xF0: ("Message 1", QColor(255, 140, 0), "l"),
    0xF1: ("Message 2", QColor(255, 165, 0), "k"),
    0xF2: ("Message 3", QColor(255, 215, 0), "n"),
    0xF3: ("Message 4", QColor(255, 255, 100), "v"),
    0xF4: ("Mini-Game", QColor(255, 105, 180), "h"),
}

# Tile grouping for palette
TILE_GROUPS = [
    ("Floor & Walls", [0x00, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE]),
    ("Exit", [0xF5]),
    ("Player", [0x01]),
    ("Enemies", [0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A]),
    ("Weapons", [0x0D, 0x0E, 0x0F]),
    ("Items", [0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x17]),
    ("Special", [0xF0, 0xF1, 0xF2, 0xF3, 0xF4]),
]

DEFAULT_TILE_CODE = 0x00
CELL_SIZE = 20
DEFAULT_MAP_SIZE = (12, 12)


class ResizeDialog(QDialog):
    def __init__(self, current_w, current_h, parent=None):
        super().__init__(parent)
        self.setWindowTitle("Map Size")
        self.setFixedSize(250, 150)
        
        layout = QFormLayout(self)
        
        self.spin_width = QSpinBox()
        self.spin_width.setRange(1, 5000)
        self.spin_width.setValue(current_w)
        
        self.spin_height = QSpinBox()
        self.spin_height.setRange(1, 5000)
        self.spin_height.setValue(current_h)
        
        layout.addRow("Width (x):", self.spin_width)
        layout.addRow("Height (y):", self.spin_height)
        
        buttons = QDialogButtonBox(QDialogButtonBox.StandardButton.Ok | 
                                   QDialogButtonBox.StandardButton.Cancel)
        buttons.accepted.connect(self.accept)
        buttons.rejected.connect(self.reject)
        layout.addRow(buttons)
    
    def get_size(self):
        return self.spin_width.value(), self.spin_height.value()


class MapCanvas(QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setMouseTracking(True)
        self.map_data = []
        self.width = 0
        self.height = 0
        self.current_tile_code = DEFAULT_TILE_CODE
        self.drawing = False
        self.resize_map(*DEFAULT_MAP_SIZE)
    
    def resize_map(self, w, h):
        self.width = w
        self.height = h
        new_map = [[DEFAULT_TILE_CODE for _ in range(w)] for _ in range(h)]
        for y in range(min(len(self.map_data), h)):
            for x in range(min(len(self.map_data[0]), w)):
                new_map[y][x] = self.map_data[y][x]
        self.map_data = new_map
        self.update()
        self.setMinimumSize(w * CELL_SIZE, h * CELL_SIZE)
    
    def clear_map(self):
        self.map_data = [[DEFAULT_TILE_CODE for _ in range(self.width)] 
                        for _ in range(self.height)]
        self.update()
    
    def paintEvent(self, event):
        painter = QPainter(self)
        painter.fillRect(self.rect(), QColor(30, 30, 30))
        
        font = QFont("Segoe UI Symbol", 12)
        font.setBold(True)
        painter.setFont(font)
        
        for y in range(self.height):
            for x in range(self.width):
                code = self.map_data[y][x]
                tile_info = TILES.get(code, ("Unknown", QColor(0, 0, 0), "?"))
                color = tile_info[1]
                symbol = tile_info[2] if len(tile_info) > 2 else "?"
                
                rect_x = x * CELL_SIZE
                rect_y = y * CELL_SIZE
                
                painter.fillRect(rect_x, rect_y, CELL_SIZE, CELL_SIZE, color)
                painter.setPen(QColor(60, 60, 60))
                painter.drawRect(rect_x, rect_y, CELL_SIZE, CELL_SIZE)
                painter.setPen(QColor(255, 255, 255))
                painter.drawText(rect_x, rect_y, CELL_SIZE, CELL_SIZE,
                               Qt.AlignmentFlag.AlignCenter, symbol)
    
    def mousePressEvent(self, event: QMouseEvent):
        if event.button() == Qt.MouseButton.LeftButton:
            self.drawing = True
            self.paint_tile(event.pos())
    
    def mouseMoveEvent(self, event: QMouseEvent):
        if self.drawing:
            self.paint_tile(event.pos())
    
    def mouseReleaseEvent(self, event: QMouseEvent):
        if event.button() == Qt.MouseButton.LeftButton:
            self.drawing = False
    
    def paint_tile(self, pos: QPoint):
        x = pos.x() // CELL_SIZE
        y = pos.y() // CELL_SIZE
        if 0 <= x < self.width and 0 <= y < self.height:
            if self.map_data[y][x] != self.current_tile_code:
                self.map_data[y][x] = self.current_tile_code
                self.update(x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE)


class TileButton(QPushButton):
    def __init__(self, code, name, color, symbol, parent_canvas):
        super().__init__(parent_canvas)
        self.code = code
        self.canvas = parent_canvas
        self.setFixedSize(180, 50)
        self.setText(f"{symbol} {name}\n0x{code:02X}")
        
        text_color = 'white' if color.lightness() < 128 else 'black'
        self.setStyleSheet(f"""
            background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 {color.name()}, stop:1 {color.darker(150).name()});
            color: {text_color};
            border: 1px solid #555;
            border-radius: 3px;
            font-size: 9px;
            text-align: left;
            padding-left: 5px;
        """)
        self.clicked.connect(self.select_tile)
    
    def select_tile(self):
        self.canvas.current_tile_code = self.code


class MapEditor(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Map Editor (PyQt6) ✦")
        self.resize(1100, 700)
        
        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        main_layout = QHBoxLayout(central_widget)
        main_layout.setContentsMargins(0, 0, 0, 0)
        
        self.scroll_area = QScrollArea()
        self.scroll_area.setWidgetResizable(True)
        self.scroll_area.setBackgroundRole(QPalette.ColorRole.Base)
        self.canvas = MapCanvas()
        self.scroll_area.setWidget(self.canvas)
        
        self.create_menu()
        
        # Palette panel
        palette_panel = QFrame()
        palette_panel.setFrameShape(QFrame.Shape.StyledPanel)
        palette_panel.setFixedWidth(280)
        palette_layout = QVBoxLayout(palette_panel)
        palette_layout.setContentsMargins(0, 0, 0, 0)
        
        header_widget = QWidget()
        header_layout = QVBoxLayout(header_widget)
        header_layout.setContentsMargins(5, 5, 5, 5)
        
        palette_label = QLabel("<b>🎨 Tile Palette:</b>")
        header_layout.addWidget(palette_label)
        
        btn_clear = QPushButton("🗑️ Clear Map")
        btn_clear.clicked.connect(self.canvas.clear_map)
        header_layout.addWidget(btn_clear)
        header_layout.addSpacing(5)
        
        palette_layout.addWidget(header_widget)
        
        scroll_palette = QScrollArea()
        scroll_palette.setWidgetResizable(True)
        scroll_palette.setHorizontalScrollBarPolicy(Qt.ScrollBarPolicy.ScrollBarAlwaysOff)
        
        scroll_content = QWidget()
        scroll_content_layout = QVBoxLayout(scroll_content)
        scroll_content_layout.setAlignment(Qt.AlignmentFlag.AlignTop)
        
        for group_name, codes in TILE_GROUPS:
            grp_lbl = QLabel(f"<b>{group_name}</b>")
            grp_lbl.setStyleSheet("margin-top: 12px; margin-bottom: 4px; padding-left: 5px; color: #ccc;")
            scroll_content_layout.addWidget(grp_lbl)
            
            line = QFrame()
            line.setFrameShape(QFrame.Shape.HLine)
            line.setFrameShadow(QFrame.Shadow.Sunken)
            line.setStyleSheet("color: #444;")
            scroll_content_layout.addWidget(line)
            
            for code in codes:
                tile_info = TILES.get(code, ("Unknown", QColor(100, 100, 100), "?"))
                name = tile_info[0]
                color = tile_info[1]
                symbol = tile_info[2] if len(tile_info) > 2 else "?"
                btn = TileButton(code, name, color, symbol, self.canvas)
                scroll_content_layout.addWidget(btn)
        
        scroll_content_layout.addStretch()
        scroll_palette.setWidget(scroll_content)
        palette_layout.addWidget(scroll_palette)
        
        main_layout.addWidget(palette_panel)
        main_layout.addWidget(self.scroll_area)
    
    def create_menu(self):
        menubar = self.menuBar()
        file_menu = menubar.addMenu('File')
        
        action_new = QAction('📐 New Size...', self)
        action_new.triggered.connect(self.action_resize_map)
        file_menu.addAction(action_new)
        
        action_save = QAction('💾 Save Map...', self)
        action_save.triggered.connect(self.action_save_map)
        file_menu.addAction(action_save)
        
        action_open = QAction('📂 Open Map...', self)
        action_open.triggered.connect(self.action_open_map)
        file_menu.addAction(action_open)
        
        file_menu.addSeparator()
        
        action_exit = QAction('🚪 Exit', self)
        action_exit.triggered.connect(self.close)
        file_menu.addAction(action_exit)
    
    def action_resize_map(self):
        dlg = ResizeDialog(self.canvas.width, self.canvas.height, self)
        if dlg.exec():
            w, h = dlg.get_size()
            self.canvas.resize_map(w, h)
    
    def action_save_map(self):
        file_path, _ = QFileDialog.getSaveFileName(self, "Save Map", "", 
                                                   "Text Files (*.txt);;All Files (*)")
        if file_path:
            try:
                with open(file_path, 'w', encoding='utf-8') as f:
                    for row in self.canvas.map_data:
                        line = ", ".join([f"0x{val:02X}" for val in row])
                        f.write(line + ",\n")
                QMessageBox.information(self, "✅ Success", "Map saved!")
            except Exception as e:
                QMessageBox.critical(self, "❌ Error", f"Failed to save file:\\n{e}")
    
    def action_open_map(self):
        file_path, _ = QFileDialog.getOpenFileName(self, "Open Map", "", 
                                                   "Text Files (*.txt);;All Files (*)")
        if file_path:
            try:
                new_map_data = []
                max_width = 0
                with open(file_path, 'r', encoding='utf-8') as f:
                    for line in f:
                        clean_line = line.strip().rstrip(',')
                        if not clean_line:
                            continue
                        parts = [p.strip() for p in clean_line.split(',')]
                        row = []
                        for part in parts:
                            if not part:
                                continue
                            val = int(part, 16)
                            row.append(val)
                        if row:
                            new_map_data.append(row)
                            if len(row) > max_width:
                                max_width = len(row)
                
                if not new_map_data:
                    QMessageBox.warning(self, "⚠️ Warning", 
                                      "File is empty or has invalid format.")
                    return
                
                height = len(new_map_data)
                for row in new_map_data:
                    while len(row) < max_width:
                        row.append(DEFAULT_TILE_CODE)
                
                self.canvas.width = max_width
                self.canvas.height = height
                self.canvas.map_data = new_map_data
                self.canvas.setMinimumSize(max_width * CELL_SIZE, height * CELL_SIZE)
                self.canvas.update()
                
                QMessageBox.information(self, "✅ Success", 
                                      f"Map loaded: {max_width}x{height}")
            except Exception as e:
                QMessageBox.critical(self, "❌ Error", 
                                   f"Failed to open file:\\n{e}")


if __name__ == '__main__':
    app = QApplication(sys.argv)
    app.setStyle("Fusion")
    window = MapEditor()
    window.show()
    sys.exit(app.exec())
