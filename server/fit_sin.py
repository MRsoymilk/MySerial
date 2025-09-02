from flask import Flask, request, jsonify
import numpy as np
from scipy.optimize import curve_fit
from scipy.signal import find_peaks
import matplotlib
matplotlib.use('Agg')  # 用于非 GUI 环境
import matplotlib.pyplot as plt
import os
import uuid

app = Flask(__name__, static_folder='static')
os.makedirs("static", exist_ok=True)

# 拟合函数
def sine_func(x, y0, A, xc, w, k, b):
    return k * (y0 + A * np.sin(np.pi * (x - xc) / w)) + b

@app.route('/find_peak', methods=['POST'])
def find_peak():
    try:
        data = request.get_json()
        x_data = np.array(data['x'], dtype=float)
        y_data = np.array(data['y'], dtype=float)

        # 找所有峰
        peaks, properties = find_peaks(y_data, prominence=0.3)  # 可以加参数 height, distance 等控制
        peak_positions = x_data[peaks]
        peak_values = y_data[peaks]

        # 如果没有峰，返回错误
        if len(peaks) == 0:
            print("No peaks found")
            return jsonify({"error": "No peaks found"}), 400

        plt.figure(figsize=(19.2 / 2, 10.8 / 2), dpi=100, facecolor='white')
        plt.plot(x_data, y_data, label="Signal")
        plt.scatter(peak_positions, peak_values, color='red', marker='x', s=100, label="Peaks")
        for px, py in zip(peak_positions, peak_values):
            plt.text(px, py,
                    f"({px:.1f}, {py:.2f})",
                    fontsize=9,
                    ha='left', va='bottom', color='blue', rotation=30)
        plt.xlabel("X Axis")
        plt.ylabel("Y Axis")
        plt.title("Peak Detection")
        plt.legend()

        uuid_tick = uuid.uuid4().hex
        filename = f"peaks_{uuid_tick}.png"
        filepath = os.path.join("static", filename)
        plt.savefig(filepath)
        plt.close()

        image_url = f"{request.host_url.rstrip('/')}/static/{filename}?t={uuid_tick}"

        # 构造返回 JSON
        return jsonify({
            "peaks": [{"x": float(px), "y": float(py)} for px, py in zip(peak_positions, peak_values)],
            "image_url": image_url
        })

    except Exception as e:
        print(f"error: {str(e)}")
        return jsonify({"error": str(e)}), 500


@app.route('/fit_sin', methods=['POST'])
def fit():
    try:
        data = request.get_json()
        x_data = np.array(data['x'], dtype=float)
        y_data = np.array(data['y'], dtype=float)
        k = float(data['k'])
        b = float(data['b'])

        # 初始猜测
        y0_init = np.mean(y_data)
        A_init = (np.max(y_data) - np.min(y_data)) / 2
        xc_init = x_data[np.argmax(y_data)]
        w_init = (np.max(x_data) - np.min(x_data)) / 2
        p0 = [y0_init, A_init, xc_init, w_init]

        # 拟合参数
        params, _ = curve_fit(
            lambda x, y0, A, xc, w: sine_func(x, y0, A, xc, w, k, b),
            x_data, y_data, p0=p0
        )
        y0, A, xc, w = params

        # 拟合曲线数据
        x_fit = np.linspace(np.min(x_data), np.max(x_data), 500)
        y_fit = sine_func(x_fit, y0, A, xc, w, k, b)

        # 生成唯一图像名
        uuid_tick = uuid.uuid4().hex
        filename = f"fit_{uuid_tick}.png"
        filepath = os.path.join('static', filename)

        # 绘图（对象式接口）
        fig, ax = plt.subplots(figsize=(19.2 / 2, 10.8 / 2), dpi=100, facecolor='white')
        ax.plot(x_data, y_data, 'o', label='Data', color='blue')
        ax.plot(x_fit, y_fit, '-', label='Fitted Curve', color='red')
        ax.set_xlabel("x")
        ax.set_ylabel("y")
        ax.grid(True)
        ax.legend()
        fig.tight_layout()
        fig.savefig(filepath, facecolor='white')
        plt.close(fig)

        # --- 计算 loss ---
        y_pred = sine_func(x_data, y0, A, xc, w, k, b)           # 拟合值对应原始 x_data
        residuals = y_data - y_pred                  # 残差
        mse = float(np.mean(residuals**2))           # 均方误差
        rss = float(np.sum(residuals**2))            # 残差平方和
        tss = float(np.sum((y_data - np.mean(y_data))**2))
        r2 = 1 - rss / tss if tss != 0 else 0.0

        # 构造返回 JSON
        image_url = f"{request.host_url.rstrip('/')}/static/{filename}?t={uuid_tick}"
        return jsonify({
            "y0": float(y0),
            "A": float(A),
            "xc": float(xc),
            "w": float(w),
            "loss_mse": mse,
            "loss_rss": rss,
            "r2": r2,
            "image_url": image_url
        })

    except Exception as e:
        return jsonify({"error": str(e)}), 500

def linear_fit_metrics(x, y):
    # 拟合
    k, b = np.polyfit(x, y, 1)
    y_pred = k * x + b
    
    # 计算误差
    residuals = y - y_pred
    mse = np.mean(residuals ** 2)
    mae = np.mean(np.abs(residuals))
    
    # 决定系数 R^2
    ss_res = np.sum(residuals ** 2)
    ss_tot = np.sum((y - np.mean(y)) ** 2)
    r2 = 1 - ss_res / ss_tot if ss_tot != 0 else float('nan')
    
    return {
        "k": float(k),
        "b": float(b),
        "mse": float(mse),
        "mae": float(mae),
        "r2": float(r2)
    }

@app.route('/fit_kb', methods=['POST'])
def fit_kb():
    data = request.get_json()
    
    temperature_data = np.array(data['temperature'], dtype=float)
    slopes_data = np.array(data['slopes'], dtype=float)
    intercepts_data = np.array(data['intercepts'], dtype=float)
    
    # 对两条曲线分别拟合
    slope_result = linear_fit_metrics(temperature_data, slopes_data)
    intercept_result = linear_fit_metrics(temperature_data, intercepts_data)
    
    result = {
        "slope_fit": slope_result,
        "intercept_fit": intercept_result
    }
    
    print(result)
    return jsonify(result)


if __name__ == '__main__':
    # 不建议生产环境使用，建议用 gunicorn 等部署
    app.run(host='0.0.0.0', port=5000, debug=True)
# Linux: gunicorn -w 4 -b 0.0.0.0:5000 fit_sin:app
# Windows: waitress-serve --listen=0.0.0.0:5000 fit_sin:app