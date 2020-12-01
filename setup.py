from setuptools import setup, find_packages
import sys, os

version = '${PROJECT_VERSION}'
requirements = """${PYTHON_REQUIREMENTS}""".split('\n')

setup(
    name='${PROJECT_NAME}',
    version=version,
    description='Tweetoscope',
    author='Anass Elidrissi',
    author_email='anasselidrissi97@gmail.com',
    url=
    'https://gitlab-student.centralesupelec.fr/tweetoscope-mullticolore/tweetou',
    packages=find_packages(),
    include_package_data=True,
    zip_safe=False,
    entry_points={
        'console_scripts': [
            'learner = ml.learner:main',
            'hawkes = ml.hawkes:main',
            'predictor = ml.predictor:main',
        ],
    },
    install_requires=requirements,
)
