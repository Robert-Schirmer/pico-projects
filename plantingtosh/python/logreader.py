import os
from typing import NamedTuple

class LogLine(NamedTuple):
    received: int
    temp: float
    capacitence: int
    plant_id: str


class BackwardsReader:
    def readline(self):
        while len(self.data) == 1 and ((self.blkcount * self.blksize) < self.size):
            self.blkcount = self.blkcount + 1
            line = self.data[0]
            try:
                self.f.seek(-self.blksize * self.blkcount, 2)  # read from end of file
                self.data = str.split(
                    str(self.f.read(self.blksize), "utf-8") + line, "\n"
                )
            except IOError:  # can't seek before the beginning of the file
                self.f.seek(0)
                self.data = str.split(
                    str(
                        self.f.read(self.size - (self.blksize * (self.blkcount - 1))),
                        "utf-8",
                    )
                    + line,
                    "\n",
                )

        if len(self.data) == 0:
            return ""

        # self.data.pop()
        # make it compatible with python <= 1.5.1
        line = self.data[-1]
        self.data = self.data[:-1]
        return self.parseline(line + "\n")

    def parseline(self, line: str) -> LogLine:
        fields = {}
        fields_list = line.strip().split(",")

        for field in fields_list:
            key, value = field.split("=")
            fields[key] = value

        return LogLine(
            received=int(fields.get("received")) / 1000,
            temp=int(fields.get("temp")) / 10,
            capacitence=int(fields.get("capacitence")),
            plant_id=fields.get("plant_id"),
        )

    def close(self):
        self.f.close()

    def __init__(self, file, blksize=4096):
        """initialize the internal structures"""
        # get the file size
        self.size = os.stat(file)[6]
        # how big of a block to read from the file...
        self.blksize = blksize
        # how many blocks we've read
        self.blkcount = 1
        self.f = open(file, "rb")
        # if the file is smaller than the blocksize, read a block,
        # otherwise, read the whole thing...
        if self.size > self.blksize:
            self.f.seek(-self.blksize * self.blkcount, 2)  # read from end of file
        self.data = str(self.f.read(self.blksize), "utf-8").split("\n")
        # strip the last item if it's empty...  a byproduct of the last line having
        # a newline at the end of it
        if not self.data[-1]:
            # self.data.pop()
            self.data = self.data[:-1]
