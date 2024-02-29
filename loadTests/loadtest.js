import http from "k6/http";
import { sleep, check } from "k6";

let target = __ENV.TARGET ? parseInt(__ENV.TARGET, 10) : 50;
let host = __ENV.HOST || "http://localhost:8081";
let action = __ENV.ACTION || "AGGREGATED";
let filter = __ENV.FILTER || "TRUE";

export let options = {
  stages: [
    { duration: "5s", target: target },
    { duration: "60s", target: target },
  ],
};

function getQuery(filter, action) {
  return {
    action: getAction(action),
    filterExpression: getFilter(filter),
  };
}

function getAction(action) {
  switch (action) {
    case "AGGREGATED":
      return {
        type: "Aggregated",
      };
    case "MUTATIONS":
      return {
        minProportion: 0.001,
        type: "Mutations",
      };
    default:
      return {
        type: "Aggregated",
      };
  }
}

function getFilter(filter) {
  switch (filter) {
    case "TRUE":
      return {
        type: "True",
      };
    case "INT_EQUALS":
      return {
        type: "IntEquals",
        column: "age",
        value: 25,
      };
    case "COMPLEX_MUTATION":
      return {
        type: "And",
        children: [
          {
            children: [
              {
                numberOfMatchers: 2,
                matchExactly: false,
                children: [
                  {
                    sequenceName: "S",
                    position: 621,
                    type: "HasAminoAcidMutation",
                  },
                  {
                    sequenceName: "S",
                    position: 622,
                    type: "HasAminoAcidMutation",
                  },
                  {
                    sequenceName: "S",
                    position: 624,
                    type: "HasAminoAcidMutation",
                  },
                  {
                    sequenceName: "S",
                    position: 625,
                    type: "HasAminoAcidMutation",
                  },
                  {
                    sequenceName: "S",
                    position: 626,
                    type: "HasAminoAcidMutation",
                  },
                  {
                    sequenceName: "S",
                    position: 627,
                    type: "HasAminoAcidMutation",
                  },
                  {
                    sequenceName: "S",
                    position: 628,
                    type: "HasAminoAcidMutation",
                  },
                  {
                    sequenceName: "S",
                    position: 631,
                    type: "HasAminoAcidMutation",
                  },
                  {
                    sequenceName: "S",
                    position: 634,
                    type: "HasAminoAcidMutation",
                  },
                  {
                    sequenceName: "S",
                    position: 638,
                    type: "HasAminoAcidMutation",
                  },
                  {
                    sequenceName: "S",
                    position: 639,
                    type: "HasAminoAcidMutation",
                  },
                  {
                    sequenceName: "S",
                    position: 640,
                    type: "HasAminoAcidMutation",
                  },
                  {
                    sequenceName: "S",
                    position: 641,
                    type: "HasAminoAcidMutation",
                  },
                  {
                    sequenceName: "S",
                    position: 642,
                    symbol: "G",
                    type: "AminoAcidEquals",
                  },
                  {
                    sequenceName: "S",
                    position: 643,
                    type: "HasAminoAcidMutation",
                  },
                  {
                    sequenceName: "S",
                    position: 643,
                    symbol: "-",
                    type: "AminoAcidEquals",
                  },
                ],
                type: "N-Of",
              },
              {
                child: { position: 897, symbol: "A", type: "NucleotideEquals" },
                type: "Not",
              },
              {
                child: {
                  position: 28256,
                  symbol: "T",
                  type: "NucleotideEquals",
                },
                type: "Not",
              },
              {
                child: {
                  sequenceName: "ORF1a",
                  position: 10,
                  symbol: "*",
                  type: "AminoAcidEquals",
                },
                type: "Not",
              },
            ],
            type: "And",
          },
          {
            type: "DateBetween",
            column: "date",
            from: "2021-03-18",
            to: "2021-03-18",
          },
        ],
      };
    default:
      return {
        type: "True",
      };
  }
}

export default function () {
  const url = `${host}/query`;

  const query = getQuery(filter, action);

  const params = {
    headers: {
      "Content-Type": "application/json",
    },
  };

  let response = http.post(url, JSON.stringify(query), params);

  check(response, {
    "is status 200": (r) => r.status === 200,
    "log response on error": (r) => {
      if (r.status !== 200) {
        console.error(
          `Request to ${url} failed. Status: ${r.status}, Body: ${r.body}`,
        );
      }
      return r.status === 200;
    },
  });

  sleep(1);
}
