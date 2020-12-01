# Tweetoscope_2020_06
## Table of contents

<details open="open">
    <summary>Table of contents</summary>
    <ol>
        <li><a href="#about-the-project">About the project</a></li>
        <li>
            <a href="#getting-started">Getting Started</a>
            <ul>
                <li><a href="#prerequisites">Prerequisites</a></li>
                <li><a href="#installation">Installation</a></li>
                <li><a href="#launching-the-project">Launching the project</a></li>
            </ul>
        </li>
        <li><a href="#documentation">Documentation</a></li>
        <li><a href="#project-structure">Project structure</a></li>
    </ol>
</details>

## About the project

This is a cool project for CentraleSupelec's SDI major.
You can find more information <a href="http://sdi.metz.centralesupelec.fr/spip.php?article25">here</a>

## Getting Started

### Prerequisites

To run this project, you primarly need Docker. 

In order to deploy the project, you need kubectl as well as a cluster on which to deploy it.

The other prerequisites are decribed below.

#### Collector and Generator
In order to compile the C++ binaries from source, you need the following:

- C++ 17 or later
- <a href="boost.org/">Boost</a>
- <a href="https://github.com/HerveFrezza-Buet/gaml">Gaml</a>
- <a href="https://downloads.apache.org/kafka/2.6.0/kafka_2.13-2.6.0.tgz">Kafka 2.6</a>
- <a href="https://github.com/mfontanini/cppkafka">CppKafka</a>

The following packages should also be installed:
```
cmake
pkg-config
zookeeper
ssl
flex
bison
librdkafka
zstd
sasl
zlib
lz4
```

Alternatively, you can use our docker image with all dependencies pre-installed:
gitlab-student.centralesupelec.fr:4567/tweetoscope-mullticolore/tweetou/3:master

Or find the packaged binaries on the <a href="https://gitlab-student.centralesupelec.fr/tweetoscope-mullticolore/tweetou/-/releases">releases</a> page. The only dependency for the binaries is rdkafka, which doesn't have a static version.
```
$ apt-get update
$ apt-get install -y librdkafka-dev
```

#### Learner, Predictor and Estimator
To run these components, you can install the requirements using pip. When in the root of the project, run:
```
$ pip3 install -r requirements.txt
```
to install the dependencies.

### Installation

#### Building the collector and generator
Clone the repository:
```
$ git clone git@gitlab-student.centralesupelec.fr:tweetoscope-mullticolore/tweetou.git
$ cd tweetou
```
You can then build the binaries using:
```
$ mkdir build
$ cd build
$ cmake ..
$ make
```
This will generate the executables in the `/build` folder.

#### Installing the python package

As usual with python, you don't need to install the package to run it. But for ease of use, you can install the package directly using pip:

```
$ pip3 install Tweetoscope-2020-06
```
You can also find the Pypi page of the project <a href="https://pypi.org/project/Tweetoscope-2020-06/">here</a>
#### Building the docker images
The docker images containing the latest version of the executables can be found at 
`gitlab-student.centralesupelec.fr:4567/tweetoscope-mullticolore/tweetou/component-name` where `component-name` is the name of the component.
To build the images manually, you can run the following, from the root of the project:
```
$ docker build -f deployments/component-name/Dockerfile -t gitlab-student.centralesupelec.fr:4567/tweetoscope-mullticolore/tweetou/component-name .
```
Where `component-name` is similarly the name of the component. The tagging is to ensure the built image works with the kubernetes deployments.

### Launching the project

To try the project locally, you can run it with the `config/development` configurations.

For the cpp components, you can do so as follows, after having compiled the project:
```
$ cd build
$ ./scripts/kafka.sh start --zooconfig ./config/development/zookeeper.properties --serverconfig ./config/development/server.properties
$ ./TweetGenerator config/development/params.config 
$ ./TweetCollector config/development/collector.ini
```

For the ML cmponents, if you have installed the pip package, you can do:
```
$ hawkes --config config/development/hawkes_config.json
$ predictor --config config/development/predictor_config.json
$ learner --config config/development/learner_config.json
```
otherwise:
```
$ python3 src/ml/hawkes --config config/development/hawkes_config.json
$ python3 src/ml/predictor --config config/development/predictor_config.json
$ python3 src/ml/learner --config config/development/learner_config.json
```
To deploy it on a cluster instead, you first need to login to the gitlab registery:
```
$ docker login gitlab-student.centralesupelec.fr:4567
$ kubectl create secret generic regcred \
    --from-file=.dockerconfigjson=path/to/.docker/config.json \
    --type=kubernetes.io/dockerconfigjson
```

Then you can run the following:
```
$ kubectl -f create deployments/tweet-collector/deployment.yaml
$ kubectl -f create deployments/tweet-generator/deployment.yaml
```

To deploy the monitoring components on a namespace, you can run
```
$ cd infrastructure
$ ./install.sh namespace
$ kubectl port-forward <grafana-pod> 3000:3000
```
You can then access grafana from `localhost:3000`.

## Documentation

You can read the documentation <a href="https://tweetoscope-mullticolore.pages-student.centralesupelec.fr/tweetou">here</a>

## Project structure

### Overall structure

The project is structured as follows:

```
├── CMakeLists.txt
├── config
│   ├── default
│   ├── deployment
│   ├── development
│   └── test
├── data
├── deployments
├── doc
├── Dockerfile
├── infrastructure
│   ├── grafana
│   ├── install.sh
│   ├── kafka
│   ├── loki
│   └── prometheus
├── README.md
├── requirements.txt
├── scripts
├── src
│   ├── cpp
│   └── ml
└── tests
    ├── cpp
    └── python
```

- config contains the configuration files for each namespace. The default configurations get copied into the other namespaces (if they don't exist) at build time.
- data contains the files for the generator.
- deployments contains the dockerfiles and kubernete configurations to deploy the project components.
- doc contains the Doxygen documentation configurtion.
- infrastructure contains the kubernetes files for the components needed to monitor the components.
- scripts contains a few useful scripts.
- tests contains the Cpp and python tests to run on the test stage.
- src contains the source files for the project.

### Cpp source files
The cpp source files are structured as follow:
```
├── CMakeLists.txt
├── Consumer.hpp
├── Logger.hpp
├── Producer.hpp
├── TweetCollector
│   ├── CMakeLists.txt
│   ├── main.cpp
│   ├── Processor.hpp
│   ├── TweetCollectorParams.hpp
│   └── TweetStreams.hpp
└── TweetGenerator
    ├── CMakeLists.txt
    ├── main.cpp
    └── TweetGenerator.hpp
```
You can find more information on the <a href="#documentation">documentation</a>
- Consumer.hpp defines a wrapper class around CppKafka's consumer for ease of use.
- Logger.hpp initialises the logger for the project and defines useful logging macros.
- Producer.hpp similarly defines a wrapper class around CppKafka's producer for ease of use.
- TweetCollector holds the code for the TweetCollector executable, used for reading tweets from kafka topics and aggregating them into cascades before publishing them.
- TweetGenerator hols the code for the TweetGenerator executable, which generates fake tweets and publishes them on a kafka topic to simulate real tweets collected from tweeter.

### ML source files
The ML source files are structured as follow:
```
├── dashboard.py
├── hawkes.py
├── __init__.py
├── learner.py
├── monitor.py
├── predictor.py
└── utils
    ├── config.py
    ├── __init__.py
    └── logger.py
```
- dashboard: Collects the alerts and highlights the potential big cascades
- hawkes: The cascade parameter estimator
- learner: The random forest learner
- monitor: Collects the metrics, shows the mean error and the mean error on the last samples
- predictor: Predicts the total number of tweets and sends the statistics and alerts
- utlls: Contains a few miscelaneous and useful functions
