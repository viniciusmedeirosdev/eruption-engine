#!/usr/bin/env python3
"""Compara duas capturas REAIS da engine (SSAO desligado x ligado).

Uso:
    python3 tools/ssao_compare.py evidencias/<pasta>/ssao-off.png evidencias/<pasta>/ssao-on.png [saida_dir]

Gera em saida_dir (padrao: pasta da captura OFF):
  - ssao-diff.png       mapa de escurecimento (OFF - ON), amplificado x8,
                        escala de cinza: preto = sem diferenca, branco = mais oclusao
  - ssao-compare.txt    metricas numericas

O mapa de diferenca e' DERIVADO das duas capturas; nao e' uma imagem gerada
pela engine e nao substitui as capturas originais.
Dependencias: Pillow e NumPy.
"""
import sys
from pathlib import Path

import numpy as np
from PIL import Image


def luminance(path: Path) -> np.ndarray:
    rgb = np.asarray(Image.open(path).convert("RGB"), dtype=np.float64) / 255.0
    return rgb @ np.array([0.2126, 0.7152, 0.0722])


def main() -> int:
    if len(sys.argv) < 3:
        print(__doc__)
        return 2
    off_path, on_path = Path(sys.argv[1]), Path(sys.argv[2])
    out_dir = Path(sys.argv[3]) if len(sys.argv) > 3 else off_path.parent
    out_dir.mkdir(parents=True, exist_ok=True)

    off, on = luminance(off_path), luminance(on_path)
    if off.shape != on.shape:
        print(f"Resolucoes diferentes: {off.shape} x {on.shape}", file=sys.stderr)
        return 1

    darker = off - on  # positivo = pixel ficou mais escuro com SSAO ligado
    h, w = off.shape
    lines = [
        f"off: {off_path}",
        f"on:  {on_path}",
        f"resolucao: {w}x{h}",
        f"luminancia media OFF: {off.mean():.5f}",
        f"luminancia media ON:  {on.mean():.5f}",
        f"escurecimento medio (OFF-ON): {darker.mean():.5f} "
        f"({100.0 * darker.mean() / max(off.mean(), 1e-9):.2f}% da luminancia OFF)",
        f"diferenca absoluta media: {np.abs(darker).mean():.5f}",
        f"percentil 99 do escurecimento: {np.percentile(darker, 99):.5f}",
        f"pixels escurecidos > 1/255: {100.0 * (darker > 1 / 255).mean():.2f}%",
        f"pixels escurecidos > 4/255: {100.0 * (darker > 4 / 255).mean():.2f}%",
        f"pixels clareados   > 4/255: {100.0 * (darker < -4 / 255).mean():.2f}%",
    ]
    (out_dir / "ssao-compare.txt").write_text("\n".join(lines) + "\n", encoding="utf-8")

    heat = np.clip(darker * 8.0, 0.0, 1.0)
    Image.fromarray((heat * 255.0 + 0.5).astype(np.uint8), mode="L").save(out_dir / "ssao-diff.png")
    print("\n".join(lines))
    print(f"mapa de diferenca: {out_dir / 'ssao-diff.png'}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
