from django.views.generic.base import TemplateView
from filereader import BackwardsReader, PlantInfos
import pytz

EST = pytz.timezone("US/Eastern")


class HomePageView(TemplateView):
    template_name = "dashboard.html"

    def get_context_data(self, **kwargs):
        context = super().get_context_data(**kwargs)

        log_files = [
            "./plant_logs/log_E6616408432F122D.txt",
            "./plant_logs/log_E6616408435E092D.txt",
        ]
        context["plant_data"] = []

        plant_infos = PlantInfos()

        for log_file in log_files:
            reader = BackwardsReader(log_file)
            log = reader.parseline(reader.readline())
            reader.close()

            plant_info = plant_infos.get(log.plant_id)

            context["plant_data"].append(
                {
                    "last_updated": log.received.astimezone(EST).strftime(
                        "%H:%M:%S %Y-%m-%d"
                    ),
                    "name": plant_info.name,
                    "type": plant_info.type,
                    "id": log.plant_id,
                    "temp": round((log.temp * 9 / 5) + 32, 1),
                    "capacitence": log.capacitence,
                }
            )

        return context
