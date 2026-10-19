#!/usr/bin/env python3
import os
import re
import time
import math
import socket
import struct
import subprocess
from collections import deque
import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d.art3d import Poly3DCollection
from matplotlib.animation import FuncAnimation

# 1. Buscar automáticamente la dirección RAM de 'g_telemetry' en el archivo .map
def find_telemetry_address():
    possible_maps = [
        "Debug/STM32F407_Tilt_System.map",
        "../Debug/STM32F407_Tilt_System.map",
        "STM32F407_Tilt_System.map"
    ]
    for path in possible_maps:
        if os.path.exists(path):
            with open(path, "r", errors="ignore") as f:
                content = f.read()
            match = re.search(r"(0x200[0-9a-fA-F]{5})\s+g_telemetry", content)
            if match:
                addr = int(match.group(1), 16)
                print(f"[OK] Símbolo 'g_telemetry' encontrado en RAM: {hex(addr)}")
                return addr
    raise RuntimeError("No se encontró 'g_telemetry' en el archivo .map. ¡Compila primero en CubeIDE!")

# 2. Cliente ligero para leer la RAM en vivo vía CoreSight AHB-AP (OpenOCD)
class CoreSightReader:
    def __init__(self, addr):
        self.addr = addr
        self.proc = subprocess.Popen(
            ["openocd", "-f", "interface/stlink.cfg", "-f", "target/stm32f4x.cfg"],
            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL
        )
        time.sleep(1.2)
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.connect(("127.0.0.1", 4444))
        self.sock.settimeout(0.2)
        self._recv()

    def _recv(self):
        data = b""
        while True:
            try:
                chunk = self.sock.recv(1024)
                if not chunk:
                    break
                data += chunk
                if b"> " in data:
                    break
            except socket.timeout:
                break
        return data.decode("ascii", errors="ignore")

    def read_telemetry(self):
        # Lee 10 halfwords (20 bytes) de golpe sin detener la CPU
        cmd = f"mdh {hex(self.addr)} 10\n"
        self.sock.sendall(cmd.encode("ascii"))
        resp = self._recv()
        hex_vals = re.findall(r":\s+((?:[0-9a-fA-F]{4}\s*)+)", resp)
        if not hex_vals:
            return None
        tokens = hex_vals[0].split()
        if len(tokens) < 10:
            return None
        raw_bytes = b"".join( int(t, 16).to_bytes(2, "little") for t in tokens[:10] )
        return struct.unpack("<hhhhhhHHHH", raw_bytes)

    def close(self):
        self.sock.close()
        self.proc.terminate()

# 3. Geometría 3D de la placa STM32F4-Discovery
def get_board_faces(roll, pitch):
    # Dimensiones de la placa
    w, l, h = 1.5, 2.2, 0.12
    corners = np.array([
        [-w, -l, -h], [w, -l, -h], [w, l, -h], [-w, l, -h],
        [-w, -l,  h], [w, -l,  h], [w, l,  h], [-w, l,  h]
    ])
    # Matrices de rotación 3D
    Rx = np.array([[1, 0, 0], [0, math.cos(pitch), -math.sin(pitch)], [0, math.sin(pitch), math.cos(pitch)]])
    Ry = np.array([[math.cos(roll), 0, math.sin(roll)], [0, 1, 0], [-math.sin(roll), 0, math.cos(roll)]])
    R = Ry @ Rx
    v = corners @ R.T
    faces = [
        [v[0], v[1], v[2], v[3]], [v[4], v[5], v[6], v[7]],
        [v[0], v[1], v[5], v[4]], [v[2], v[3], v[7], v[6]],
        [v[1], v[2], v[6], v[5]], [v[4], v[7], v[3], v[0]]
    ]
    # Posición de los 4 LEDs sobre la cara superior
    leds_local = np.array([
        [-1.1,  0.0, h + 0.05], # Verde (-X / Izquierda)
        [ 0.0,  1.5, h + 0.05], # Naranja (+Y / Arriba)
        [ 1.1,  0.0, h + 0.05], # Rojo (+X / Derecha)
        [ 0.0, -1.5, h + 0.05]  # Azul (-Y / Abajo)
    ])
    return faces, leds_local @ R.T

def main():
    addr = find_telemetry_address()
    reader = CoreSightReader(addr)

    plt.style.use("dark_background")
    fig = plt.figure(figsize=(13, 6))
    fig.canvas.manager.set_window_title("STM32F407 Bare-Metal + FreeRTOS | CoreSight Live Telemetry")

    # Panel Izquierdo: Gemelo Digital 3D
    ax3d = fig.add_subplot(1, 2, 1, projection="3d")
    # Panel Derecho: Osciloscopio DSP (Crudo vs Filtrado)
    ax_osc = fig.add_subplot(1, 2, 2)

    history_len = 120
    raw_x_hist  = deque([0]*history_len, maxlen=history_len)
    filt_x_hist = deque([0]*history_len, maxlen=history_len)

    line_raw,  = ax_osc.plot([], [], color="#ff4d4d", alpha=0.55, lw=1.2, label="SPI Raw X (Ruido MEMS)")
    line_filt, = ax_osc.plot([], [], color="#00ffcc", lw=2.2, label="Filtro IIR Coma Fija (>> 2)")
    ax_osc.set_xlim(0, history_len)
    ax_osc.set_ylim(-18000, 18000)
    ax_osc.set_title("Osciloscopio DSP en Tiempo Real (Eje X)", fontsize=12, fontweight="bold")
    ax_osc.set_xlabel("Muestras (50 Hz)")
    ax_osc.set_ylabel("Aceleración Cruda (LSB 16-bit)")
    ax_osc.legend(loc="upper right")
    ax_osc.grid(True, alpha=0.2)

    def update(_):
        data = reader.read_telemetry()
        if not data:
            return
        rx, ry, rz, fx, fy, fz, pg, po, pr, pb = data

        # Actualizar osciloscopio
        raw_x_hist.append(rx)
        filt_x_hist.append(fx)
        line_raw.set_data(range(len(raw_x_hist)), raw_x_hist)
        line_filt.set_data(range(len(filt_x_hist)), filt_x_hist)

        # Calcular ángulos Roll y Pitch desde los datos filtrados
        roll  = math.atan2(fx, math.sqrt(fy*fy + fz*fz + 1.0))
        pitch = math.atan2(fy, math.sqrt(fx*fx + fz*fz + 1.0))

        # Dibujar placa 3D
        ax3d.cla()
        ax3d.set_xlim(-2.2, 2.2)
        ax3d.set_ylim(-2.2, 2.2)
        ax3d.set_zlim(-1.8, 1.8)
        ax3d.set_axis_off()
        deg_r, deg_p = math.degrees(roll), math.degrees(pitch)
        ax3d.set_title(f"Gemelo Digital 3D STM32F4-Discovery\nRoll: {deg_r:+.1f}° | Pitch: {deg_p:+.1f}°", fontsize=12, fontweight="bold")

        faces, leds_pos = get_board_faces(roll, -pitch)
        poly = Poly3DCollection(faces, facecolors="#006644", edgecolors="#00ffaa", linewidths=1.2, alpha=0.85)
        ax3d.add_collection3d(poly)

        # Dibujar los 4 LEDs sobre la placa 3D brillando según el PWM real del TIM4
        colors = ["#00ff00", "#ff8800", "#ff0000", "#0088ff"]
        pwms   = [pg, po, pr, pb]
        for i in range(4):
            intensity = 0.2 + 0.8 * (pwms[i] / 1000.0)
            size = 60 + 180 * (pwms[i] / 1000.0)
            ax3d.scatter(leds_pos[i,0], leds_pos[i,1], leds_pos[i,2],
                         color=colors[i], alpha=intensity, s=size, edgecolors="white")

    ani = FuncAnimation(fig, update, interval=25, cache_frame_data=False)
    try:
        plt.tight_layout()
        plt.show()
    finally:
        reader.close()

if __name__ == "__main__":
    main()
