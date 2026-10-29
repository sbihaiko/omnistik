#!/usr/bin/env python3
"""Harness de bancada da sonda Omnistik (spec 1, ticket 2).

Verifica, do lado do host, o que o firmware promete:

- um único dispositivo USB pai expondo N interfaces HID de gamepad;
- o descritor de relatório declarado, salvo em disco;
- relatórios chegando em toda interface, no layout combinado
  (16 botões em 2 bytes + 4 eixos de 8 bits, neutro 128);
- sem vazamento: a interface i só pressiona o botão i.

Uso:
    tools/.venv/bin/python tools/hid_harness.py --pads 1 --seconds 6 --out out

Saída: <out>/descriptor-if<N>.hex e <out>/resumo.json. Código de saída 0 só
quando todas as checagens passam.
"""

import argparse
import json
import queue
import sys
import threading
import time
from pathlib import Path

import hid

DEFAULT_VID = 0xCAFE
DEFAULT_PID = 0x4001
AXES_NEUTRAL = 128


def find_interfaces(vid, pid):
    return [d for d in hid.enumerate(vid, pid)]


def abrir(device_path):
    """Abre uma interface pela API antiga do hidapi (hid.device)."""
    dev = hid.device()
    dev.open_path(device_path)
    return dev


def observe(device_path, seconds, out_queue):
    """Abre uma interface e coleta (timestamp, relatório) até o prazo."""
    try:
        dev = abrir(device_path)
    except Exception as exc:  # noqa: BLE001 - qualquer falha de abertura conta
        out_queue.put(("erro", str(exc)))
        return

    deadline = time.monotonic() + seconds
    try:
        dev.set_nonblocking(0)
        while time.monotonic() < deadline:
            data = dev.read(64, 200)
            if data:
                out_queue.put(("relatorio", list(data)))
    except Exception as exc:  # noqa: BLE001
        out_queue.put(("erro", str(exc)))
    finally:
        dev.close()


def normalize(report):
    """O backend do macOS entrega o Report ID como primeiro byte."""
    if len(report) == 7 and report[0] == 0x01:
        return report[1:]
    return report


def check_report(report, index):
    """Devolve a lista de problemas do relatório para a interface `index`."""
    problemas = []
    report = normalize(report)
    if len(report) != 6:
        problemas.append(f"tamanho {len(report)}, esperado 6")
        return problemas

    botoes = report[0] | (report[1] << 8)
    eixos = report[2:6]
    if any(v != AXES_NEUTRAL for v in eixos):
        problemas.append(f"eixos fora do neutro: {eixos}")

    permitido = 0 if report == [0, 0] + [AXES_NEUTRAL] * 4 else (1 << index)
    if botoes & ~permitido:
        problemas.append(
            f"botão de outra interface: 0x{botoes:04x} (esperado 0 ou 0x{permitido:04x})"
        )
    return problemas


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--vid", type=lambda s: int(s, 0), default=DEFAULT_VID)
    ap.add_argument("--pid", type=lambda s: int(s, 0), default=DEFAULT_PID)
    ap.add_argument("--pads", type=int, default=1, help="interfaces esperadas")
    ap.add_argument("--seconds", type=float, default=6.0)
    ap.add_argument("--out", type=Path, default=Path("out"))
    args = ap.parse_args()

    args.out.mkdir(parents=True, exist_ok=True)
    resumo = {"vendor": hex(args.vid), "product": hex(args.pid), "esperado": args.pads}
    falhas = []

    interfaces = find_interfaces(args.vid, args.pid)
    resumo["interfaces_encontradas"] = len(interfaces)
    if len(interfaces) != args.pads:
        falhas.append(
            f"esperadas {args.pads} interfaces, encontradas {len(interfaces)}"
        )

    seriais = {i.get("serial_number") for i in interfaces}
    resumo["seriais"] = sorted(s for s in seriais if s)
    if len(seriais) > 1:
        falhas.append(f"interfaces com seriais diferentes: {resumo['seriais']}")

    if not interfaces:
        (args.out / "resumo.json").write_text(json.dumps(resumo, indent=2))
        print(json.dumps(resumo, indent=2, ensure_ascii=False))
        print("FALHA: dispositivo não encontrado", file=sys.stderr)
        return 1

    descritores = {}
    for index, info in enumerate(interfaces):
        try:
            dev = abrir(info["path"])
            try:
                desc = bytes(dev.get_report_descriptor())
            finally:
                dev.close()
        except Exception:  # noqa: BLE001 - alguns backends não expõem
            desc = b""
        descritores[index] = desc
        (args.out / f"descriptor-if{index}.hex").write_text(desc.hex() + "\n")

    resumo["descritores_distintos"] = len({d for d in descritores.values() if d})

    filas = []
    threads = []
    for info in interfaces:
        fila = queue.Queue()
        filas.append(fila)
        t = threading.Thread(target=observe, args=(info["path"], args.seconds, fila))
        t.start()
        threads.append(t)
    for t in threads:
        t.join()

    por_interface = []
    for index, fila in enumerate(filas):
        itens = []
        while not fila.empty():
            itens.append(fila.get())
        erros = [msg for tipo, msg in itens if tipo == "erro"]
        relatorios = [r for tipo, r in itens if tipo == "relatorio"]

        problemas = []
        if erros:
            problemas.extend(erros)
        if not relatorios:
            problemas.append("nenhum relatório recebido")
        for rel in relatorios:
            problemas.extend(check_report(rel, index))

        normalizados = [normalize(r) for r in relatorios]
        bits = sorted({r[0] | (r[1] << 8) for r in normalizados if len(r) == 6})
        por_interface.append(
            {
                "interface": index,
                "relatorios": len(relatorios),
                "botoes_observados": [hex(b) for b in bits],
                "problemas": problemas,
            }
        )
        problemas = list(dict.fromkeys(problemas))
        falhas.extend(f"interface {index}: {p}" for p in problemas)

    resumo["interfaces"] = por_interface
    resumo["ok"] = not falhas
    resumo["falhas"] = falhas

    (args.out / "resumo.json").write_text(json.dumps(resumo, indent=2, ensure_ascii=False))
    print(json.dumps(resumo, indent=2, ensure_ascii=False))
    return 0 if not falhas else 1


if __name__ == "__main__":
    sys.exit(main())
