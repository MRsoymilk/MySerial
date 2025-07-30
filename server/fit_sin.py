from flask import Flask, request, jsonify
import numpy as np
from scipy.optimize import curve_fit
import matplotlib
matplotlib.use('Agg')  # 用于非 GUI 环境
import matplotlib.pyplot as plt
import os
import uuid

app = Flask(__name__, static_folder='static')
os.makedirs("static", exist_ok=True)

# 拟合函数
def sine_func(x, y0, A, xc, w):
    return y0 + A * np.sin(np.pi * (x - xc) / w)

@app.route('/hello')
def hello():
    return "hello"

@app.route('/fit_sin', methods=['POST'])
def fit():
    try:
        data = request.get_json()
        x_data = np.array(data['x'], dtype=float)
        y_data = np.array(data['y'], dtype=float)

        # 初始猜测
        y0_init = np.mean(y_data)
        A_init = (np.max(y_data) - np.min(y_data)) / 2
        xc_init = x_data[np.argmax(y_data)]
        w_init = (np.max(x_data) - np.min(x_data)) / 2
        p0 = [y0_init, A_init, xc_init, w_init]

        # 拟合参数
        params, _ = curve_fit(sine_func, x_data, y_data, p0=p0)
        y0, A, xc, w = params

        # 拟合曲线数据
        x_fit = np.linspace(np.min(x_data), np.max(x_data), 500)
        y_fit = sine_func(x_fit, *params)

        # 生成唯一图像名
        filename = f"fit_{uuid.uuid4().hex}.png"
        filepath = os.path.join('static', filename)

        # 绘图（对象式接口）
        fig, ax = plt.subplots(figsize=(19.2, 10.8), dpi=100, facecolor='white')
        ax.plot(x_data, y_data, 'o', label='Data', color='blue')
        ax.plot(x_fit, y_fit, '-', label='Fitted Curve', color='red')
        ax.set_xlabel("x")
        ax.set_ylabel("y")
        ax.grid(True)
        ax.legend()
        fig.tight_layout()
        fig.savefig(filepath, facecolor='white')
        plt.close(fig)

        # 构造返回 JSON
        image_url = f"{request.host_url.rstrip('/')}/static/{filename}?t={uuid.uuid4().hex}"
        return jsonify({
            "y0": float(y0),
            "A": float(A),
            "xc": float(xc),
            "w": float(w),
            "image_url": image_url
        })

    except Exception as e:
        return jsonify({"error": str(e)}), 500

if __name__ == '__main__':
    # 不建议生产环境使用，建议用 gunicorn 等部署
    app.run(host='0.0.0.0', port=5000, debug=True)
# Linux: gunicorn -w 4 -b 0.0.0.0:5000 fit_sin:app
# Windows: Windows: waitress-serve --listen=0.0.0.0:5000 fit_sin:app
