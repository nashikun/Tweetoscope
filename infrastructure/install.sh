#!/bin/bash

start_deployments ()
{
    echo "Starting grafana"
    kubectl apply -k grafana -n $1
    echo "Starting loki"
    kubectl apply -k loki -n $1
    echo "Starting prometheus"
    kubectl apply -k prometheus -n $1
    echo "Starting kafka"
    kubectl apply -k kafka
}

stop_deployments ()
{
    echo "Stopping grafana"
    kubectl delete -k grafana -n $1
    echo "Stopping loki"
    kubectl delete -k loki -n $1
    echo "Stopping prometheus"
    kubectl delete -k prometheus -n $1
    echo "Stopping kafka"
    kubectl delete -k kafka
}

if [ $# -lt 1 ]; then
    echo "Please enter a namespace"
    exit 1
fi

stop_deployments $1
start_deployments $1
