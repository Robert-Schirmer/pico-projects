import numpy as np
from process_log import process_log


def celcius_to_fahrenheit(celcius):
    return celcius * 9 / 5 + 32


def linspace_datetime64(start_date, end_date, n):
    return np.linspace(0, 1, n) * (end_date - start_date) + start_date


class PlantStats:
    def __init__(self, log_file: str):
        self.log_file = log_file

    def load(self, days_back: int):
        self.data_points = process_log(self.log_file, days_back)

    def last_log(self):
        return {
            "temp": self.data_points.temp[-1],
            "capacitence": self.data_points.capacitence[-1],
            "timestamp": self.data_points.timestamps[-1],
        }

    def get_data_points(self):
        return self.data_points
