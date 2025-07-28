from typing import NamedTuple
import matplotlib.pyplot as plt
import matplotlib.dates as md
import datetime
import numpy as np
from scipy.interpolate import interp1d
from scipy import signal
import time
import sys
from logreader import BackwardsReader


def celcius_to_fahrenheit(celcius):
    return celcius * 9 / 5 + 32


def linspace_datetime64(start_date, end_date, n):
    return np.linspace(0, 1, n) * (end_date - start_date) + start_date


class PlantDataPoints(NamedTuple):
    water_markers: list
    resampled_timestamps: list
    timestamps: list
    temp_smooth: list
    temp: list
    capacitence_smooth: list
    capacitence: list


def process_log(log_file, read_back_days=10):
    cleaned_data = []
    bad_data = []
    timestamps = []

    start = time.time()

    backwards_reader = BackwardsReader(log_file)

    end_timestamp = int(
        (datetime.datetime.now() - datetime.timedelta(read_back_days)).strftime("%s")
    )

    done = False
    while not done:
        line = backwards_reader.readline()
        if not line:
            done = True
            continue

        temp = line.temp

        if temp < 2 or temp > 400:
            bad_data.append(line)
            continue

        capacitence = line.capacitence

        if capacitence < 100 or capacitence > 1000:
            bad_data.append(line)
            continue

        timestamp = line.received

        if timestamp < end_timestamp:
            done = True
            print(f"Found end timestamp: {timestamp}")
            continue

        timestamps.append(timestamp)
        cleaned_data.append(line)

    backwards_reader.close()

    cleaned_data.reverse()
    timestamps.reverse()

    end = time.time()

    print(f"Read {len(cleaned_data)} records in {end - start} s from {log_file}")
    print(f"Found {len(bad_data)} bad records.")

    capacitence = [line.capacitence for line in cleaned_data]
    temp = [celcius_to_fahrenheit(line.temp) for line in cleaned_data]

    seconds_of_data = timestamps[-1] - timestamps[0]
    avg_sample_rate_hz = len(timestamps) / seconds_of_data
    print(f"Sample rate: {round(avg_sample_rate_hz, 2)} Hz")
    avg_samples_per_hour = avg_sample_rate_hz * 60 * 60
    print(f"Samples per hour: {round(avg_samples_per_hour, 2)}")

    # Perform filtering of signal to smooth out noise
    window_size = round(avg_samples_per_hour / 2)
    window_size = window_size + 1 if window_size % 2 == 0 else window_size

    temp_filtered = signal.savgol_filter(temp, window_size, 1)
    capacitence_filtered = signal.savgol_filter(capacitence, window_size, 1)

    # Perform interpolation
    f_temp = interp1d(timestamps, temp_filtered, kind="linear")
    f_capacitence = interp1d(timestamps, capacitence_filtered, kind="linear")

    # Evenly spaced timestamps for resampling
    resampled_timestamps = linspace_datetime64(
        timestamps[0], timestamps[-1], int((timestamps[-1] - timestamps[0]) / (60 * 60))
    )
    temp_smooth = f_temp(resampled_timestamps)
    capacitence_smooth = f_capacitence(resampled_timestamps)
    d_capacitence_smooth = np.gradient(capacitence_smooth)

    # Amount of time to wait before marking another water event
    water_time_cooldown = 60 * 60 * 3  # seconds
    last_water_time = 0  # seconds
    # Amount of change in capacitence to trigger a water event
    d_capacitence_for_watering = 5

    water_markers = []

    for i in range(0, len(capacitence_smooth)):
        if (
            d_capacitence_smooth[i] > d_capacitence_for_watering
            and (resampled_timestamps[i] - last_water_time) > water_time_cooldown
        ):
            water_markers.append(capacitence_smooth[i])
            print(
                f"Watered at {datetime.datetime.fromtimestamp(resampled_timestamps[i])}, dCapacitence: {d_capacitence_smooth[i]}"
            )
            last_water_time = resampled_timestamps[i]
        else:
            water_markers.append(None)

    return PlantDataPoints(
        water_markers,
        resampled_timestamps.tolist(),
        timestamps,
        temp_smooth.tolist(),
        temp,
        capacitence_smooth.tolist(),
        capacitence,
    )

if __name__ == "__main__":
    log_file = f"./plant_logs/log_{sys.argv[1]}.txt"
    days_back = int(sys.argv[2]) if len(sys.argv) >= 3 else 10

    plant_data_points = process_log(log_file, days_back)

    # Plot
    plot_raw = True  # Plot the raw data points
    fig, ax1 = plt.subplots()

    ax = plt.gca()
    xfmt = md.DateFormatter("%Y-%m-%d %H:%M:%S")
    ax.xaxis.set_major_formatter(xfmt)
    plt.setp(ax.get_xticklabels(), rotation=30, horizontalalignment="right")
    ax.xaxis.set_visible(False)

    resampled_datetime = [datetime.datetime.fromtimestamp(x) for x in plant_data_points.resampled_timestamps]
    timestamps_datetime = [datetime.datetime.fromtimestamp(x) for x in plant_data_points.timestamps]

    color = "tab:red"
    ax1.set_xlabel("time")
    ax1.set_ylabel("temp (F)", color=color)
    ax1.plot(resampled_datetime, plant_data_points.temp_smooth, color=color)
    if plot_raw:
        ax1.scatter(timestamps_datetime, plant_data_points.temp, color=color, alpha=0.1, s=1)

    ax1.tick_params(axis="y", labelcolor=color)

    ax2 = ax1.twinx()

    color = "tab:blue"
    ax2.set_ylabel("capacitence", color=color)
    ax2.plot(resampled_datetime, plant_data_points.capacitence_smooth, color=color)
    if plot_raw:
        ax2.scatter(timestamps_datetime, plant_data_points.capacitence, color=color, alpha=0.1, s=1)
    ax2.tick_params(axis="y", labelcolor=color)

    ax2.plot(resampled_datetime, plant_data_points.water_markers, "go")

    fig.tight_layout()
    plt.show()
