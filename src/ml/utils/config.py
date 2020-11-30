import json


def init_config(args):
    """Reads the configuration from the path while overwriting any prmeters by those given in the config
    """

    with open(args.config, 'r') as f:
        config = json.load(f)
    for k, v in vars(args).items():
        if k != "config":
            config[k] = v
    return config
