import argparse
import pandas as pd
import numpy as np
from scipy.interpolate import UnivariateSpline, splrep, PPoly
import matplotlib.pyplot as plt
import os


def main():
    parser = argparse.ArgumentParser(description="Spline interpolation for spectrum")

    parser.add_argument("--input", required=True, help="input csv file")
    parser.add_argument("--output", default="test_threshold.csv", help="output csv file")

    parser.add_argument("--start", type=float, default=900)
    parser.add_argument("--end", type=float, default=1700)
    parser.add_argument("--step", type=float, default=1)

    parser.add_argument("--smooth", type=float, default=0.5)

    parser.add_argument("--mode", choices=["round", "floor"], default="round")

    parser.add_argument("--plot", action="store_true")
    parser.add_argument("--export-poly", action="store_true")

    args = parser.parse_args()

    # =========================
    # 读取数据
    # =========================
    input_path = os.path.expanduser(args.input)
    df = pd.read_csv(input_path)

    x = df["Wavelength"].values
    y = df["Intensity"].values

    # =========================
    # 拟合
    # =========================
    spline = UnivariateSpline(x, y, s=args.smooth * len(x))

    # =========================
    # 生成新波长
    # =========================
    x_new = np.arange(args.start, args.end + args.step, args.step)

    # =========================
    # 插值
    # =========================
    y_new = spline(x_new)

    # =========================
    # 转整数
    # =========================
    if args.mode == "round":
        y_int = np.rint(y_new).astype(int)
    else:
        y_int = np.floor(y_new).astype(int)

    # =========================
    # 保存
    # =========================
    result = pd.DataFrame({
        "wavelength": x_new,
        "threshold": y_int
    })

    result.to_csv(args.output, index=False, encoding="utf-8-sig")
    print("已生成：", args.output)

    # =========================
    # 可视化
    # =========================
    if args.plot:
        plt.figure()
        plt.scatter(x, y, s=2, label="raw")
        plt.plot(x_new, y_new, label="fit")
        plt.plot(x_new, y_int, ".", label="int")
        plt.legend()
        plt.grid()
        plt.show()

    # =========================
    # 导出分段多项式
    # =========================
    if args.export_poly:
        tck = splrep(x, y, s=args.smooth * len(x))
        pp = PPoly.from_spline(tck)

        xs = pp.x
        cs = pp.c

        for i in range(len(xs) - 1):
            a, b, c, d = cs[:, i]
            print(f"区间 [{xs[i]:.2f}, {xs[i+1]:.2f}] :")
            print(f"y = {a:.6e} x^3 + {b:.6e} x^2 + {c:.6e} x + {d:.6e}")
            print()


if __name__ == "__main__":
    main()
