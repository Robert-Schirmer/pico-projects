import json
from typing import NamedTuple
from plantstats import PlantStats

class PlantInfo(NamedTuple):
    name: str
    type: str
    stats: PlantStats


class PlantInfos:
    def __init__(self):
        f = open("./plant_logs/plant_info.json")
        plant_info = json.load(f)
        f.close()
        plant_info_dict: dict[str, PlantInfo] = {}
        for key, value in plant_info.items():
            plant_info_dict[key] = PlantInfo(
                name=value["name"],
                type=value["type"],
                stats=PlantStats(f"./plant_logs/{value["log_file"]}"),
            )
        self.plant_info_dict = plant_info_dict
    
    def get_plant_ids(self) -> list[str]:
        return list(self.plant_info_dict.keys())

    def get(self, plant_id: str) -> PlantInfo:
        return self.plant_info_dict.get(plant_id, None)
