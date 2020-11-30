from setuptools import setup, find_packages
import sys, os

with open('requirements.txt', 'r') as f:
    reqs = f.read()

version = '${PROJECT_VERSION}'

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
        ],
    },
    install_requires=reqs,
)
