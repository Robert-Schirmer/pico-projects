import datetime
from django.views.generic.base import TemplateView
from plantinfos import PlantInfos
import pytz

EST = pytz.timezone("US/Eastern")


class HomePageView(TemplateView):
    template_name = "dashboard.html"

    def get_context_data(self, **kwargs):
        context = super().get_context_data(**kwargs)

        load_days = self.get_days_to_load()

        context["plant_data"] = []

        plant_infos = PlantInfos()

        for plant_id in plant_infos.get_plant_ids():
            plant_info = plant_infos.get(plant_id)

            plant_info.stats.load(load_days)

            last_log = plant_info.stats.last_log()
            data_points = plant_info.stats.get_data_points()

            context["plant_data"].append(
                {
                    "last_updated": last_log.get("timestamp"),
                    "name": plant_info.name,
                    "type": plant_info.type,
                    "id": plant_id,
                    "temp": round(last_log.get("temp"), 1),
                    "temp_series": data_points.temp_smooth,
                    "capacitence": last_log.get("capacitence"),
                    "capacitence_series": data_points.capacitence_smooth,
                    "time_series": data_points.resampled_timestamps,
                    "water_markers": data_points.water_markers,
                }
            )

        return context

    def get_days_to_load(self) -> int:
        try:
            return int(self.request.GET.get("days"))
        except:
            return 21
