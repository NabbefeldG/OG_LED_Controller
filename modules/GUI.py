from PyQt5.QtCore import Qt, QTimer
from PyQt5.QtWidgets import QWidget, QGridLayout, QLabel, QSlider, QPushButton, QCheckBox, QLineEdit
from modules.Serial_functions import search_for_teensy_module, send_data_until_confirmation

test_mode = False
LED_INTENSITY = 126  # sends two values defining og0.intensity and og1.intensity

SWITCH_CW_ON_OG0 = 127  # Not triggerable anymore. Line is constantly on at the set intensity.
SWITCH_CW_OFF_OG0 = 128  # Switches CW mode off again. To normal state.
SWITCH_CW_ON_OG1 = 129  # Not triggerable anymore. Line is constantly on at the set intensity.
SWITCH_CW_OFF_OG1 = 130  # Switches CW mode off again. To normal state.
MODULE_INFO = 255  # returns module information


class Window(QWidget):
    def __init__(self, parent=None, teensy_name='OG_LED_Module', data_transmit_rate=10):
        # Setup window
        super(Window, self).__init__(parent)
        self.setWindowTitle('LED Controller')
        self.setGeometry(200, 400, 250, 100)

        # Define serial object and default values
        if not test_mode:
            self.serial_obj = search_for_teensy_module(teensy_name)
            send_data_until_confirmation(serial_obj=self.serial_obj, header_byte=LED_INTENSITY,
                                         data=[0.0, 0.0])
        #

        self.blue_LED_value = 0.0
        self.uv_LED_value = 0.0
        self.value_change = True

        # placeholder for gui elements
        self.blue_LED_label = None
        self.blue_LED = None
        self.uv_LED_label = None
        self.uv_LED = None

        self.OG0_CW_checkBox = None
        self.OG1_CW_checkBox = None

        # generate Widgets
        self.build_gui()

        # Add a timer to regulate the data flow to the teensy
        self.timer = QTimer()
        self.timer.timeout.connect(self.timeout)
        self.timer.start(int(1000 / data_transmit_rate))
    #

    def build_gui(self):
        grid = QGridLayout()
        self.blue_LED_label = QLabel('OG0 intensity: 0.00')
        grid.addWidget(self.blue_LED_label, 0, 0)

        self.blue_LED = QLineEdit('0.0')
        self.blue_LED.textChanged.connect(lambda: self.intensity_callback('og0'))
        self.blue_LED.setText(str(self.blue_LED_value))
        grid.addWidget(self.blue_LED, 1, 0)

        self.uv_LED_label = QLabel('OG1 intensity: 0.00')
        grid.addWidget(self.uv_LED_label, 2, 0)

        self.uv_LED = QLineEdit('0.0')
        self.uv_LED.textChanged.connect(lambda: self.intensity_callback('og1'))
        self.uv_LED.setText(str(self.uv_LED_value))
        grid.addWidget(self.uv_LED, 3, 0)

        self.OG0_CW_checkBox = QCheckBox('OG0 CW_mode')
        self.OG0_CW_checkBox.setCheckable(True)
        self.OG0_CW_checkBox.stateChanged.connect(lambda: self.checkbox_callback('og0'))
        self.OG0_CW_checkBox.checkState()
        grid.addWidget(self.OG0_CW_checkBox, 0, 1, 2, 1)

        self.OG1_CW_checkBox = QCheckBox('OG1 CW_mode')
        self.OG1_CW_checkBox.setCheckable(True)
        self.OG1_CW_checkBox.stateChanged.connect(lambda: self.checkbox_callback('og1'))
        self.OG1_CW_checkBox.checkState()
        grid.addWidget(self.OG1_CW_checkBox, 2, 1, 2, 1)

        self.setLayout(grid)
    #

    def intensity_callback(self, parent_name):
        if parent_name == 'og0':
            try:
                self.blue_LED_value = float(self.blue_LED.text())
                # self.blue_LED.setText('%0.3f' % self.blue_LED_value)
            except:
                pass
            #

            if self.blue_LED_value < 0:
                self.blue_LED_value = 0
            elif self.blue_LED_value > 1:
                self.blue_LED_value = 1
            #

            self.blue_LED_label.setText('OG0 intensity: %0.3f' % self.blue_LED_value)
        if parent_name == 'og1':
            try:
                self.uv_LED_value = float(self.uv_LED.text())
                # self.uv_LED.setText('%0.3f' % self.uv_LED_value)
            except:
                pass
            #

            if self.uv_LED_value < 0:
                self.uv_LED_value = 0
            elif self.uv_LED_value > 1:
                self.uv_LED_value = 1
            #

            self.uv_LED_label.setText('OG1 intensity: %0.3f' % self.uv_LED_value)
        #
        self.value_change = True
    #

    def checkbox_callback(self, parent_name):
        if parent_name == 'og0':
            if self.OG0_CW_checkBox.checkState() > 0:
                # Switch to CW_ON
                send_data_until_confirmation(serial_obj=self.serial_obj, header_byte=SWITCH_CW_ON_OG0)
            else:
                # OFF
                send_data_until_confirmation(serial_obj=self.serial_obj, header_byte=SWITCH_CW_OFF_OG0)
            #
        else:
            if self.OG1_CW_checkBox.checkState() > 0:
                # Switch to CW_ON
                send_data_until_confirmation(serial_obj=self.serial_obj, header_byte=SWITCH_CW_ON_OG1)
            else:
                # OFF
                send_data_until_confirmation(serial_obj=self.serial_obj, header_byte=SWITCH_CW_OFF_OG1)
            #
        #
    #

    def timeout(self):
        if self.value_change:
            self.blue_LED_label.setText('OG0 intensity: %0.3f' % self.blue_LED_value)
            self.uv_LED_label.setText('OG1 intensity: %0.3f' % self.uv_LED_value)

            if not test_mode:
                print([self.blue_LED_value, self.uv_LED_value])
                send_data_until_confirmation(serial_obj=self.serial_obj, header_byte=LED_INTENSITY,
                                             data=[self.blue_LED_value, self.uv_LED_value])
            else:
                print([self.blue_LED_value, self.uv_LED_value])
            #

            self.value_change = False
        #
    #
#
