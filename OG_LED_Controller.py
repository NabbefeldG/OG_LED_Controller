import sys
from PyQt5.QtWidgets import QApplication
from modules.GUI import Window
import serial


if __name__ == '__main__':
    app = QApplication(sys.argv)
    gui = Window(teensy_name='OG_LED_Module', data_transmit_rate=10)  # Maximum transmit rate in Hz
    gui.show()
    sys.exit(app.exec_())
#
