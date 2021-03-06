<div>
<img src="https://user-images.githubusercontent.com/1674891/43786026-f2043344-9a67-11e8-8434-27c324c37214.png" width="60%"></img>
<h1>Simple. Adaptive. Neural.</h1>
<br>
<div/>

We think that artificial intelligence is going to be the next big thing in our nearby future. It will bring humanity to a new era of access and organization of information.
Language translation is probably the most complex of the human tasks for a machine to learn but it is also the one with the greatest potential to make the world a single family.

With this project we want to give our contribution to the evolution of machine translation toward singularity.
We want to consolidate the current state of the art into a single easy to use product, evolve it and keeping it an open to integrate the greatest opportunities in machine intelligence like deep learning.

To achieve our goals we need a better MT technology that is able to extract more from data, adapt to context and be easy to deploy. We know that the challenge is big, but the reward is potentially so big that we think it is worth trying hard.

## About ModernMT
ModernMT is a context-aware, incremental and distributed general purpose Neural Machine Translation technology based on **Tensorflow Transformer model**. ModernMT is:
- Easy to use and scale with respect to domains, data, and users.
- Trained by pooling all available projects/customers data and translation memories in one folder.
- Queried by providing the sentence to be translated and optionally some context text.

ModernMT goal is to deliver the quality of multiple custom engines by adapting on the fly to the provided context.

You can find more information on: http://www.modernmt.eu

## Your first translation with ModernMT

### Installation

Read [INSTALL.md](INSTALL.md)

The distribution includes a small dataset (folder `examples/data/train`) to train and test translations from 
English to Italian in three domains. 

### Create an engine

We will now demonstrate how easy it is to train your first engine with MMT. *Please notice* however that the provided training set is tiny and exclusively intended for this demo. If you wish to train a proper engine please follow the instructions provided in this guide: [Create an engine from scratch](https://github.com/ModernMT/MMT/wiki/Create-an-engine-from-scratch).

Creating an engine in ModernMT is this simple:
```bash
$ ./mmt create en it examples/data/train --train-steps 6000
```

This command will start a fast training process that will last approximately 30mins; not enough to achieve good translation performance, but enough to demonstrate its functioning.

### Start the engine

```bash
$ ./mmt start
```
You can stop it with the command `stop`.

### Start translating

Let's now use the command-line tool `mmt` to query the engine with the sentence *hello world* and context *computer*:
```
$ ./mmt translate "This is an example"
Si tratta di un esempio
```

*Note:* You can query MMT directly via REST API, to learn more on how to do it, visit the [Translate API](https://github.com/ModernMT/MMT/wiki/API-Translate) page in the project Wiki.


### How to import a TMX file

Importing a TMX file is very simple and fast. We will use again the command-line tool `mmt`:
```
$ ./mmt import -x /path/to/file.tmx
Importing example... [========================================] 100.0% 00:35
IMPORT SUCCESS
```

## Evaluating quality

How is your engine performing vs the commercial state-of-the-art technologies?

Should I use Google Translate or ModernMT given this data? 

Evaluate helps you answer these questions.

Before training, MMT has removed sentences corresponding to 1% of the training set (or up to 1200 lines at most).
During evaluate these sentences are used to compute the BLUE Score and Matecat Post-Editing Score against the MMT and Google Translate.

With your engine running, just type:
```
./mmt evaluate
```
The typical output will be
```
Testing on 980 sentences...

Matecat Post-Editing Score:
  MMT              : 75.10 (Winner)
  Google Translate : 73.40 | API Limit Exeeded | Connection Error

BLEU:
  MMT              : 37.50 (Winner)
  Google Translate : 36.10 | API Limit Exeeded | Connection Error

Translation Speed:
  MMT              :  1.75s per sentence
  Google Translate :  0.76s per sentence
  
```

If you want to test on a different test-set just type:
```
./mmt evaluate --path path/to/your/test-set
```

*Notes:* To run Evaluate you need internet connection for Google Translate API and the Matecat Post-Editing Score API.
MMT comes with a limited Google Translate API key. 

Matecat kindly provides unlimited-fair-usage, access to their API to MMT users.

You can select your Google Translate API Key by typing:
```
./mmt evaluate --gt-key YOUR_GOOGLE_TRANSLATE_API_KEY
```

If you don't want to use Google Translate just type a random key.

## What's next?

#### Create an engine from scratch
Following this README you have learned the basic usage of ModernMT. Most users would be interested in creating their own engine with their own data, you can find more info in the Wiki [Create an engine from scratch](https://github.com/ModernMT/MMT/wiki/Create-an-engine-from-scratch)

#### See API Documentation
ModernMT comes with built-in REST API that allows the user to control every single feature of MMT via a simple and powerful interface. You can find the [API Documentation](https://github.com/ModernMT/MMT/wiki/API-Documentation) in the [ModernMT Wiki](https://github.com/ModernMT/MMT/wiki).

#### Run ModernMT cluster
You can setup a cluster of MMT nodes in order to load balancing translation requests. In fact also tuning and evaluation can be drastically speed-up if runned on an MMT cluster.
You can learn more on the Wiki page [MMT Cluster](https://github.com/ModernMT/MMT/wiki/MMT-Cluster).

#### Use advanced configurations
If you need to customize the properties and behaviour of your engines, you can specify advanced settings in their configuration files. 
You can learn how on the Wiki page [Advanced Configurations](https://github.com/ModernMT/MMT/wiki/Advanced-Configurations)

# Enterprise Edition

MMT Community is free, is open source and welcomes contributions and donations.
MMT Community is sponsored by its funding members (Translated, FBK, UEDIN and TAUS) and the European Commission. 

We also have an **MMT Enterprise Edition**, managed by the MMT company and not available on GitHub, with some extra features:
- Pre-trained generic and custom models in 45 language pairs (and more to come!) on multiple billion words of premium data.
- Support for cluster of servers for higher throughput, load balancing and high availability. 
- Support for 71 files formats without format loss (Office, Adobe, Localization, etc).
- Enterprise Customer Support via Video Conference Call, Phone and Email on business hours (CET) and optionally 24x7.
- Custom developments billed per hour of work.

For any information please email us at info@modernmt.eu
