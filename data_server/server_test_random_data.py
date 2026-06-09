# Public Invention's Ventmon-Ventilator-Inline-Test-Monitor Project is an attempt
# to build a "test fixture" capable of running a 48-hour test on any ventilator
# design and collecting data on many important parameters. We hope to create a
# "gold standard" test that all DIY teams can work to; but this project will
# proceed in parallel with that. The idea is to make a standalone inline device
# plugged into the airway. It serves a dual purpose as a monitor/alarm when used
# on an actual patient, and a test device for testing prototype ventilators. It
# also allows for burnin. Copyright (C) 2021 Robert L. Read, Lauria Clarke,
# Ben Coombs, Darío Hereñú, and Geoff Mulligan.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.

"""Send randomized VentMon measurements to a data-lake TCP endpoint."""

from __future__ import annotations

import random
import socket
import sys
import time
import traceback
from dataclasses import dataclass

DATA_LAKE_HOST = "ventmon.coslabs.com"
DATA_LAKE_PORT = 6110
DEFAULT_SAMPLE_COUNT = 0

DEVICE_TYPES = ("B", "A", "M", "D")
MEASUREMENT_TYPES = ("T", "P", "D", "F", "O", "H", "V", "B", "G", "A")


@dataclass(frozen=True)
class Measurement:
    """A single VentMon measurement encoded in the PIRDS binary format."""

    measurement_type: str
    device_type: str
    device_location: int
    measurement_time: int
    measurement_value: int

    def as_bytes(self) -> bytearray:
        """Return this measurement as a 12-byte PIRDS measurement frame."""
        encoded = bytearray(12)
        encoded[0] = ord("M")
        encoded[1] = ord(self.measurement_type)
        encoded[2] = ord(self.device_type)
        encoded[3] = self.device_location
        encoded[4:8] = self.measurement_time.to_bytes(4, "big")
        encoded[8:12] = self.measurement_value.to_bytes(4, "big", signed=True)
        return encoded

    def asBytes(self) -> bytearray:
        """Compatibility wrapper for the original camelCase method name."""
        return self.as_bytes()

    def print_measurement(self) -> None:
        print(
            "measurement to send: ",
            "M",
            self.measurement_type,
            self.device_type,
            self.device_location,
            self.measurement_time,
            self.measurement_value,
        )

    def printMeasurement(self) -> None:
        """Compatibility wrapper for the original camelCase method name."""
        self.print_measurement()


def terminate_with_newline(measurement_bytes: bytearray) -> bytearray:
    """Append the newline byte expected by the data-lake socket server."""
    return measurement_bytes + b"\n"


def random_measurement(start_time: int) -> Measurement:
    """Build a randomized measurement relative to ``start_time`` in ms."""
    return Measurement(
        measurement_type=random.choice(MEASUREMENT_TYPES),
        device_type=random.choice(DEVICE_TYPES),
        device_location=random.randrange(0, 10),
        measurement_time=int(round(time.time() * 1000)) - start_time,
        measurement_value=random.randrange(-10, 10),
    )


def send_random_measurements(sock: socket.socket, sample_count: int) -> int:
    """Send ``sample_count`` randomized measurements and return the sent count."""
    start_time = int(round(time.time() * 1000))
    sent_count = 0

    while sample_count <= 0 or sent_count < sample_count:
        measurement = random_measurement(start_time)
        print(f"sending {sent_count} / {sample_count}")

        try:
            payload = terminate_with_newline(measurement.as_bytes())
            measurement.print_measurement()
            print(f'sending "{payload}"\n')
            sock.sendall(payload)
            sent_count += 1
        except OverflowError:
            print("Overflow error:", sys.exc_info()[0])
            traceback.print_exc(file=sys.stdout)
            break
        except BrokenPipeError:
            traceback.print_exc(file=sys.stdout)
            break

    return sent_count


def parse_args(argv: list[str]) -> tuple[str, int, int]:
    """Parse positional CLI args while preserving the legacy interface."""
    host = argv[1] if len(argv) > 1 else DATA_LAKE_HOST
    port = int(argv[2]) if len(argv) > 2 else DATA_LAKE_PORT
    sample_count = int(argv[3]) if len(argv) > 3 else DEFAULT_SAMPLE_COUNT
    return host, port, sample_count


def main(argv: list[str] | None = None) -> int:
    """CLI entry point."""
    argv = argv or sys.argv
    print(f"Arguments count: {len(argv)}")
    print(f"Name of the script      : {argv[0]}")
    print(f"Arguments of the script : {argv[1:]}\n")

    host, port, sample_count = parse_args(argv)
    server_address = (host, port)

    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
            sock.connect(server_address)
            sent_count = send_random_measurements(sock, sample_count)
            print("\nclosing socket")
            print(f"sent: {sent_count} / {sample_count}")
    except ConnectionRefusedError:
        print(f"Connection refused: {server_address}")
        return 1

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
