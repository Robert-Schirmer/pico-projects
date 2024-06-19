import matplotlib.pyplot as plt
import matplotlib.dates as md
import datetime
import numpy as np
from scipy.interpolate import interp1d
from scipy import signal

def celcius_to_fahrenheit(celcius):
  return celcius * 9 / 5 + 32

def linspace_datetime64(start_date, end_date, n):
    return np.linspace(0, 1, n) * (end_date - start_date) + start_date

cleaned_data = []
bad_data = []

with open("log.txt") as f:
  for line in f:
    fields = {}
    fields_list = line.strip().split(',')
    
    for field in fields_list:
      key, value = field.split('=')
      fields[key] = value
    
    if len(fields.get('received')) != len('1718406938144'):
      bad_data.append(fields)
      continue
    
    temp = int(fields.get('temp'))
    
    if temp < 2 or temp >400:
      bad_data.append(fields)
      continue
    
    capacitence = int(fields.get('capacitence'))
    
    if capacitence < 100 or capacitence > 600:
      bad_data.append(fields)
      continue
      
    # Only one plant for now with this ID, so all data is for this plant
    if fields.get('plant_id') != 'E6616408435E092D':
      fields['plant_id'] = 'E6616408435E092D'
    
    cleaned_data.append(fields)
    
print(f"Found {len(bad_data)} bad records.")
print(bad_data)

# with open("cleaned_data.txt", "w") as f:
#   for fields in cleaned_data:
#     line = ",".join([f"{key}={value}" for key, value in fields.items()])
#     f.write(line + "\n")

timestamps = [int(fields.get('received')) / 1000 for fields in cleaned_data]

capacitence = [int(fields.get('capacitence')) for fields in cleaned_data]
temp = [celcius_to_fahrenheit(int(fields.get('temp')) / 10) for fields in cleaned_data]

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
f_temp = interp1d(timestamps, temp_filtered, kind='linear')
f_capacitence = interp1d(timestamps, capacitence_filtered, kind='linear')

# Evenly spaced timestamps for resampling
resampled_timestamps = linspace_datetime64(timestamps[0], timestamps[-1], int((timestamps[-1] - timestamps[0]) / (60 * 60)))
temp_smooth = f_temp(resampled_timestamps)
capacitence_smooth = f_capacitence(resampled_timestamps)

# Plot
plot_raw = True # Plot the raw data points
fig, ax1 = plt.subplots()

ax=plt.gca()
xfmt = md.DateFormatter('%Y-%m-%d %H:%M:%S')
ax.xaxis.set_major_formatter(xfmt)

resampled_datetime = [datetime.datetime.fromtimestamp(x) for x in resampled_timestamps]
timestamps_datetime = [datetime.datetime.fromtimestamp(x) for x in timestamps]

color = 'tab:red'
ax1.set_xlabel('time')
ax1.set_ylabel('temp (F)', color=color)
ax1.plot(resampled_datetime, temp_smooth, color=color)
if (plot_raw):
  ax1.scatter(timestamps_datetime, temp, color=color, alpha=0.1, s=1)
ax1.tick_params(axis='y', labelcolor=color)

ax2 = ax1.twinx()

color = 'tab:blue'
ax2.set_ylabel('capacitence', color=color)
ax2.plot(resampled_datetime, capacitence_smooth, color=color)
if (plot_raw):
  ax2.scatter(timestamps_datetime, capacitence, color=color, alpha=0.1, s=1)
ax2.tick_params(axis='y', labelcolor=color)

fig.tight_layout()
plt.show()
