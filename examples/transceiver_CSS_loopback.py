#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#
# SPDX-License-Identifier: GPL-3.0
#
# GNU Radio Python Flow Graph
# Title: IEEE 802.15.4 Transceiver using CSS PHY
# Description: IEEE 802.15.4 Transceiver using CSS PHY
# GNU Radio version: 3.8.1.0

from distutils.version import StrictVersion

if __name__ == '__main__':
    import ctypes
    import sys
    if sys.platform.startswith('linux'):
        try:
            x11 = ctypes.cdll.LoadLibrary('libX11.so')
            x11.XInitThreads()
        except:
            print("Warning: failed to XInitThreads()")

import os
import sys
sys.path.append(os.environ.get('GRC_HIER_PATH', os.path.expanduser('~/.grc_gnuradio')))

from PyQt5 import Qt
from PyQt5.QtCore import QObject, pyqtSlot
from gnuradio import qtgui
from gnuradio.filter import firdes
import sip
from gnuradio import analog
from gnuradio import blocks
import pmt
from gnuradio import gr
import signal
from argparse import ArgumentParser
from gnuradio.eng_arg import eng_float, intx
from gnuradio import eng_notation
from gnuradio.qtgui import Range, RangeWidget
from ieee802_15_4_css_phy import ieee802_15_4_css_phy  # grc-generated hier_block
import ieee802_15_4
import numpy as np

from gnuradio import qtgui

class transceiver_CSS_loopback(gr.top_block, Qt.QWidget):

    def __init__(self):
        gr.top_block.__init__(self, "IEEE 802.15.4 Transceiver using CSS PHY")
        Qt.QWidget.__init__(self)
        self.setWindowTitle("IEEE 802.15.4 Transceiver using CSS PHY")
        qtgui.util.check_set_qss()
        try:
            self.setWindowIcon(Qt.QIcon.fromTheme('gnuradio-grc'))
        except:
            pass
        self.top_scroll_layout = Qt.QVBoxLayout()
        self.setLayout(self.top_scroll_layout)
        self.top_scroll = Qt.QScrollArea()
        self.top_scroll.setFrameStyle(Qt.QFrame.NoFrame)
        self.top_scroll_layout.addWidget(self.top_scroll)
        self.top_scroll.setWidgetResizable(True)
        self.top_widget = Qt.QWidget()
        self.top_scroll.setWidget(self.top_widget)
        self.top_layout = Qt.QVBoxLayout(self.top_widget)
        self.top_grid_layout = Qt.QGridLayout()
        self.top_layout.addLayout(self.top_grid_layout)

        self.settings = Qt.QSettings("GNU Radio", "transceiver_CSS_loopback")

        try:
            if StrictVersion(Qt.qVersion()) < StrictVersion("5.0.0"):
                self.restoreGeometry(self.settings.value("geometry").toByteArray())
            else:
                self.restoreGeometry(self.settings.value("geometry"))
        except:
            pass

        ##################################################
        # Variables
        ##################################################
        self.text_msg = text_msg = "This is a test message"
        self.samp_rate = samp_rate = 32e6
        self.phi = phi = 0
        self.pad_len = pad_len = 0
        self.interval = interval = 300
        self.freq_off = freq_off = 0
        self.enable = enable = 1.0
        self.delay = delay = 500
        self.c = c = ieee802_15_4.css_phy(chirp_number=4, phy_packetsize_bytes=len(text_msg)+15)
        self.ampl = ampl = 0.02

        ##################################################
        # Blocks
        ##################################################
        self._phi_range = Range(-np.pi, np.pi, 0.01, 0, 200)
        self._phi_win = RangeWidget(self._phi_range, self.set_phi, 'phi', "counter_slider", float)
        self.top_grid_layout.addWidget(self._phi_win)
        self._pad_len_range = Range(0, 10000, 1, 0, 200)
        self._pad_len_win = RangeWidget(self._pad_len_range, self.set_pad_len, 'pad_len', "counter_slider", int)
        self.top_grid_layout.addWidget(self._pad_len_win)
        self._interval_range = Range(10, 1000, 1, 300, 200)
        self._interval_win = RangeWidget(self._interval_range, self.set_interval, 'interval', "counter_slider", int)
        self.top_grid_layout.addWidget(self._interval_win)
        self._freq_off_range = Range(-5e6, 5e6, 1e5, 0, 200)
        self._freq_off_win = RangeWidget(self._freq_off_range, self.set_freq_off, 'freq_off', "counter_slider", float)
        self.top_grid_layout.addWidget(self._freq_off_win)
        # Create the options list
        self._enable_options = (0.0, 1.0, )
        # Create the labels list
        self._enable_labels = ("off", "on", )
        # Create the combo box
        self._enable_tool_bar = Qt.QToolBar(self)
        self._enable_tool_bar.addWidget(Qt.QLabel('enable' + ": "))
        self._enable_combo_box = Qt.QComboBox()
        self._enable_tool_bar.addWidget(self._enable_combo_box)
        for _label in self._enable_labels: self._enable_combo_box.addItem(_label)
        self._enable_callback = lambda i: Qt.QMetaObject.invokeMethod(self._enable_combo_box, "setCurrentIndex", Qt.Q_ARG("int", self._enable_options.index(i)))
        self._enable_callback(self.enable)
        self._enable_combo_box.currentIndexChanged.connect(
            lambda i: self.set_enable(self._enable_options[i]))
        # Create the radio buttons
        self.top_grid_layout.addWidget(self._enable_tool_bar)
        self._delay_range = Range(0, 100000, 10, 500, 200)
        self._delay_win = RangeWidget(self._delay_range, self.set_delay, 'delay', "counter_slider", float)
        self.top_grid_layout.addWidget(self._delay_win)
        self._ampl_range = Range(0, 1.0, 0.01, 0.02, 200)
        self._ampl_win = RangeWidget(self._ampl_range, self.set_ampl, 'ampl', "counter_slider", float)
        self.top_grid_layout.addWidget(self._ampl_win)
        self.qtgui_waterfall_sink_x_0 = qtgui.waterfall_sink_c(
            1024, #size
            firdes.WIN_BLACKMAN_hARRIS, #wintype
            0, #fc
            samp_rate, #bw
            "", #name
            1 #number of inputs
        )
        self.qtgui_waterfall_sink_x_0.set_update_time(0.10)
        self.qtgui_waterfall_sink_x_0.enable_grid(False)
        self.qtgui_waterfall_sink_x_0.enable_axis_labels(True)



        labels = ['', '', '', '', '',
                  '', '', '', '', '']
        colors = [0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0]
        alphas = [1.0, 1.0, 1.0, 1.0, 1.0,
                  1.0, 1.0, 1.0, 1.0, 1.0]

        for i in range(1):
            if len(labels[i]) == 0:
                self.qtgui_waterfall_sink_x_0.set_line_label(i, "Data {0}".format(i))
            else:
                self.qtgui_waterfall_sink_x_0.set_line_label(i, labels[i])
            self.qtgui_waterfall_sink_x_0.set_color_map(i, colors[i])
            self.qtgui_waterfall_sink_x_0.set_line_alpha(i, alphas[i])

        self.qtgui_waterfall_sink_x_0.set_intensity_range(-140, 10)

        self._qtgui_waterfall_sink_x_0_win = sip.wrapinstance(self.qtgui_waterfall_sink_x_0.pyqwidget(), Qt.QWidget)
        self.top_grid_layout.addWidget(self._qtgui_waterfall_sink_x_0_win)
        self.qtgui_freq_sink_x_0 = qtgui.freq_sink_c(
            1024, #size
            firdes.WIN_BLACKMAN_hARRIS, #wintype
            2405000000, #fc
            samp_rate, #bw
            "", #name
            1
        )
        self.qtgui_freq_sink_x_0.set_update_time(0.10)
        self.qtgui_freq_sink_x_0.set_y_axis(-140, 10)
        self.qtgui_freq_sink_x_0.set_y_label('Relative Gain', 'dB')
        self.qtgui_freq_sink_x_0.set_trigger_mode(qtgui.TRIG_MODE_FREE, 0.0, 0, "")
        self.qtgui_freq_sink_x_0.enable_autoscale(False)
        self.qtgui_freq_sink_x_0.enable_grid(False)
        self.qtgui_freq_sink_x_0.set_fft_average(1.0)
        self.qtgui_freq_sink_x_0.enable_axis_labels(True)
        self.qtgui_freq_sink_x_0.enable_control_panel(False)



        labels = ['', '', '', '', '',
            '', '', '', '', '']
        widths = [1, 1, 1, 1, 1,
            1, 1, 1, 1, 1]
        colors = ["blue", "red", "green", "black", "cyan",
            "magenta", "yellow", "dark red", "dark green", "dark blue"]
        alphas = [1.0, 1.0, 1.0, 1.0, 1.0,
            1.0, 1.0, 1.0, 1.0, 1.0]

        for i in range(1):
            if len(labels[i]) == 0:
                self.qtgui_freq_sink_x_0.set_line_label(i, "Data {0}".format(i))
            else:
                self.qtgui_freq_sink_x_0.set_line_label(i, labels[i])
            self.qtgui_freq_sink_x_0.set_line_width(i, widths[i])
            self.qtgui_freq_sink_x_0.set_line_color(i, colors[i])
            self.qtgui_freq_sink_x_0.set_line_alpha(i, alphas[i])

        self._qtgui_freq_sink_x_0_win = sip.wrapinstance(self.qtgui_freq_sink_x_0.pyqwidget(), Qt.QWidget)
        self.top_grid_layout.addWidget(self._qtgui_freq_sink_x_0_win)
        self.ieee802_15_4_rime_stack_0 = ieee802_15_4.rime_stack([129], [131], [132], [23,42])
        self.ieee802_15_4_mac_0 = ieee802_15_4.mac(True,0x8841,0,0x1aaa,0xffff,0x3344)
        self.ieee802_15_4_css_phy_1 = ieee802_15_4_css_phy(
            bits_per_cw=c.bits_per_symbol,
            chirp_seq=c.chirp_seq,
            codewords=c.codewords,
            intlv_seq=c.intlv_seq,
            len_sub=38,
            nbytes_payload=c.phy_packetsize_bytes,
            nsamp_frame=c.nsamp_frame,
            num_subchirps=c.n_subchirps,
            nzeros_padding=c.padded_zeros,
            phr=c.PHR,
            preamble=c.preamble,
            sfd=c.SFD,
            sym_per_frame=c.nsym_frame,
            threshold=0.95,
            time_gap_1=c.time_gap_1,
            time_gap_2=c.time_gap_2,
        )
        self.blocks_vector_insert_x_0 = blocks.vector_insert_c([0 for i in range(pad_len)], 3*c.nsamp_frame, 3*c.nsamp_frame)
        self.blocks_socket_pdu_0_0 = blocks.socket_pdu('TCP_SERVER', '', '52001', 10000, False)
        self.blocks_multiply_xx_1 = blocks.multiply_vcc(1)
        self.blocks_multiply_xx_0 = blocks.multiply_vcc(1)
        self.blocks_message_strobe_0 = blocks.message_strobe(pmt.intern(text_msg), interval)
        self.blocks_delay_0 = blocks.delay(gr.sizeof_gr_complex*1, int(delay))
        self.blocks_add_xx_0 = blocks.add_vcc(1)
        self.analog_sig_source_x_0 = analog.sig_source_c(samp_rate, analog.GR_SIN_WAVE, freq_off, 1, 0, 0)
        self.analog_fastnoise_source_x_0 = analog.fastnoise_source_c(analog.GR_GAUSSIAN, ampl, 0, 8192)
        self.analog_const_source_x_0 = analog.sig_source_c(0, analog.GR_CONST_WAVE, 0, 0, np.exp(1j*phi)*enable)



        ##################################################
        # Connections
        ##################################################
        self.msg_connect((self.blocks_message_strobe_0, 'strobe'), (self.ieee802_15_4_rime_stack_0, 'bcin'))
        self.msg_connect((self.blocks_socket_pdu_0_0, 'pdus'), (self.ieee802_15_4_rime_stack_0, 'bcin'))
        self.msg_connect((self.ieee802_15_4_css_phy_1, 'rxout'), (self.ieee802_15_4_mac_0, 'pdu in'))
        self.msg_connect((self.ieee802_15_4_mac_0, 'pdu out'), (self.ieee802_15_4_css_phy_1, 'txin'))
        self.msg_connect((self.ieee802_15_4_mac_0, 'app out'), (self.ieee802_15_4_rime_stack_0, 'fromMAC'))
        self.msg_connect((self.ieee802_15_4_rime_stack_0, 'bcout'), (self.blocks_socket_pdu_0_0, 'pdus'))
        self.msg_connect((self.ieee802_15_4_rime_stack_0, 'toMAC'), (self.ieee802_15_4_mac_0, 'app in'))
        self.connect((self.analog_const_source_x_0, 0), (self.blocks_delay_0, 0))
        self.connect((self.analog_fastnoise_source_x_0, 0), (self.blocks_add_xx_0, 0))
        self.connect((self.analog_sig_source_x_0, 0), (self.blocks_multiply_xx_0, 0))
        self.connect((self.blocks_add_xx_0, 0), (self.ieee802_15_4_css_phy_1, 0))
        self.connect((self.blocks_add_xx_0, 0), (self.qtgui_freq_sink_x_0, 0))
        self.connect((self.blocks_add_xx_0, 0), (self.qtgui_waterfall_sink_x_0, 0))
        self.connect((self.blocks_delay_0, 0), (self.blocks_multiply_xx_1, 0))
        self.connect((self.blocks_multiply_xx_0, 0), (self.blocks_vector_insert_x_0, 0))
        self.connect((self.blocks_multiply_xx_1, 0), (self.blocks_add_xx_0, 1))
        self.connect((self.blocks_vector_insert_x_0, 0), (self.blocks_multiply_xx_1, 1))
        self.connect((self.ieee802_15_4_css_phy_1, 0), (self.blocks_multiply_xx_0, 1))


    def closeEvent(self, event):
        self.settings = Qt.QSettings("GNU Radio", "transceiver_CSS_loopback")
        self.settings.setValue("geometry", self.saveGeometry())
        event.accept()

    def get_text_msg(self):
        return self.text_msg

    def set_text_msg(self, text_msg):
        self.text_msg = text_msg
        self.set_c(ieee802_15_4.css_phy(chirp_number=4, phy_packetsize_bytes=len(self.text_msg)+15))
        self.blocks_message_strobe_0.set_msg(pmt.intern(self.text_msg))

    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate
        self.analog_sig_source_x_0.set_sampling_freq(self.samp_rate)
        self.qtgui_freq_sink_x_0.set_frequency_range(2405000000, self.samp_rate)
        self.qtgui_waterfall_sink_x_0.set_frequency_range(0, self.samp_rate)

    def get_phi(self):
        return self.phi

    def set_phi(self, phi):
        self.phi = phi
        self.analog_const_source_x_0.set_offset(np.exp(1j*self.phi)*self.enable)

    def get_pad_len(self):
        return self.pad_len

    def set_pad_len(self, pad_len):
        self.pad_len = pad_len

    def get_interval(self):
        return self.interval

    def set_interval(self, interval):
        self.interval = interval
        self.blocks_message_strobe_0.set_period(self.interval)

    def get_freq_off(self):
        return self.freq_off

    def set_freq_off(self, freq_off):
        self.freq_off = freq_off
        self.analog_sig_source_x_0.set_frequency(self.freq_off)

    def get_enable(self):
        return self.enable

    def set_enable(self, enable):
        self.enable = enable
        self._enable_callback(self.enable)
        self.analog_const_source_x_0.set_offset(np.exp(1j*self.phi)*self.enable)

    def get_delay(self):
        return self.delay

    def set_delay(self, delay):
        self.delay = delay
        self.blocks_delay_0.set_dly(int(self.delay))

    def get_c(self):
        return self.c

    def set_c(self, c):
        self.c = c

    def get_ampl(self):
        return self.ampl

    def set_ampl(self, ampl):
        self.ampl = ampl
        self.analog_fastnoise_source_x_0.set_amplitude(self.ampl)





def main(top_block_cls=transceiver_CSS_loopback, options=None):
    if gr.enable_realtime_scheduling() != gr.RT_OK:
        print("Error: failed to enable real-time scheduling.")

    if StrictVersion("4.5.0") <= StrictVersion(Qt.qVersion()) < StrictVersion("5.0.0"):
        style = gr.prefs().get_string('qtgui', 'style', 'raster')
        Qt.QApplication.setGraphicsSystem(style)
    qapp = Qt.QApplication(sys.argv)

    tb = top_block_cls()

    tb.start()

    tb.show()

    def sig_handler(sig=None, frame=None):
        Qt.QApplication.quit()

    signal.signal(signal.SIGINT, sig_handler)
    signal.signal(signal.SIGTERM, sig_handler)

    timer = Qt.QTimer()
    timer.start(500)
    timer.timeout.connect(lambda: None)

    def quitting():
        tb.stop()
        tb.wait()

    qapp.aboutToQuit.connect(quitting)
    qapp.exec_()

if __name__ == '__main__':
    main()
