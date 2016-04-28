import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
colors = [
    "#e91e63",
    "#f29312",
    "#4caf50",
    "#0ccff0",
    "#2c2ff0",
    "#8c2ff0",
]
d = pd.read_table("data-2.txt", names=["m", "n", "t"])
d = d.groupby(["m", "n"], as_index=False).mean()
for i, (x, s) in enumerate(d.groupby("n")):
    plt.plot(s["m"], s["t"], label="{0} processes".format(x),
             marker="x", linewidth=2, color=colors[i])
plt.plot(s["m"], s["m"] ** 3 * 8e-9 / 3, label="1GB/s model".format(x),
         linestyle="--", linewidth=2, color="grey")
plt.xlabel("M")
plt.ylabel("time /s")
plt.legend(loc="lower right")
plt.gca().set_xscale("log")
plt.gca().set_yscale("log")
plt.show()
