import argparse
import logging
import os
import subprocess
from dataclasses import dataclass
from enum import Enum

logging.basicConfig(level=logging.INFO)


class TestActions(Enum):
    AGGREGATED = 1
    MUTATIONS = 2


class TestFilters(Enum):
    TRUE = 1
    INT_EQUALS = 2
    COMPLEX_MUTATION = 3


@dataclass
class ParsedArgs:
    action: TestActions
    filter: TestFilters
    url: str
    workers: int


def parse_args() -> ParsedArgs:
    parser = argparse.ArgumentParser(description="SILO load test")

    parser.add_argument(
        '--url',
        type=str,
        default='http://localhost:8091',
        help='URL to SILO'
    )

    parser.add_argument('--workers', type=int, default=1, help='Number of workers sending requests')
    parser.add_argument('--action', type=str, default='AGGREGATED',
                        help='Query to send to SILO. Valid actions are: AGGREGATED, MUTATIONS')
    parser.add_argument('--filter', type=str, default='TRUE',
                        help='Filter for query. Valid filters are: TRUE, INT_EQUALS, COMPLEX_MUTATION')

    args = parser.parse_args()

    def action_type(arg: str) -> TestActions:
        try:
            return TestActions[arg.upper()]
        except KeyError:
            raise ValueError(f"Invalid action: {arg}. Valid actions are: AGGREGATED, MUTATIONS")

    def filter_type(arg: str) -> TestFilters:
        try:
            return TestFilters[arg.upper()]
        except KeyError:
            raise ValueError(f"Invalid filter: {arg}. Valid filters are: TRUE, INT_EQUALS, COMPLEX_MUTATION")

    return ParsedArgs(
        action=action_type(args.action),
        filter=filter_type(args.filter),
        url=args.url,
        workers=args.workers
    )


def run_load_test(args: ParsedArgs):
    logging.info(f"Running load test with args: {args}")

    container_name = f"k6_load_test"

    working_directory_of_script = os.path.dirname(os.path.realpath(__file__))

    environment_variables = f"-e TARGET={args.workers} -e HOST={args.url} -e ACTION={args.action.name} -e FILTER={args.filter.name}"

    docker_command = f"docker run --network='host' --name={container_name} -v {working_directory_of_script}:/scripts loadimpact/k6 run /scripts/loadtest.js {environment_variables}"

    logging.info(f"Executing k6 load test in Docker with command: {docker_command}")

    process = subprocess.Popen(docker_command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                               preexec_fn=os.setsid)

    try:
        stdout, stderr = process.communicate()
        if process.returncode == 0:
            logging.info("k6 load test in Docker executed successfully")
            logging.info(stdout.decode())
        else:
            logging.error("Error executing k6 load test in Docker")
            logging.error(stderr.decode())
    except KeyboardInterrupt:
        stop_command = f"docker stop {container_name}"
        subprocess.call(stop_command.split())
        logging.info("Load test interrupted by user and container stopped")


if __name__ == '__main__':
    try:
        args = parse_args()
        run_load_test(args)
    except KeyboardInterrupt:
        logging.info("Script execution interrupted by user")
