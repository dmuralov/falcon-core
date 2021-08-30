Falcon Core integration/staging tree
=====================================

https://falcon.io

What is Falcon?
----------------

Falcon is a decentralized, open source privacy platform for trading goods and services without intermediaries. The ecosystem currently includes the digital currency PART and the Falcon Marketplace application.

PART is the cryptocurrency of the Falcon blockchain. It is an independent usable secure and confidential digital currency specifically designed to power a marketplace and acts as its settlement layer. It uses several security, encryption, and privacy protocols to make sure no personal information and data can be collected when transacting on the marketplace.

|Properties|Values|
|:-------------------------|:-----------------------------------------|
|Native blockchain|Falcon|
|Blockchain codebase|Bitcoin (latest)|
|Block Time|120 seconds|
|Block Size|2 MB|
|Consensus Mechanism|Falcon Proof-of-Stake (PPoS)|
|Privacy Protocols|Confidential Transactions (CT) and RingCT|
|Bulletproofs|:white_check_mark: yes|
|Stealth Addresses|:white_check_mark: yes|
|Ring Signatures|:white_check_mark: yes|
|Cold Staking|:white_check_mark: yes|
|Segwit|:white_check_mark: yes|
|Lightning Network|:white_check_mark: yes|
|Atomic Swaps|:white_check_mark: yes|

For more information please visit https://falcon.io and https://falcon.wiki. The Falcon Desktop application is available in this repository https://github.com/falcon/falcon-desktop.

Getting Started
---------------

A new Falcon wallet will need an HD master key loaded and an initial account
derived before it will be functional.

The GUI programs will guide you through the initial setup.

It is recommended to use a mnemonic passphrase.
To generate a new passphrase see the mnemonic rpc command.
Loading the new mnemonic with the extkeyimportmaster command will setup the
master HD key and first account.

To create an initial new HD master key and account from random data, start
falcond or falcon-qt with the parameter: -createdefaultmasterkey.

Remember to backup your passphrase and/or wallet.dat file!

License
-------

Falcon Core is released under the terms of the MIT license. See [COPYING](COPYING) for more
information or see https://opensource.org/licenses/MIT.

Development Process
-------------------

The `master` branch is regularly built (see `doc/build-*.md` for instructions) and tested, but it is not guaranteed to be
completely stable. [Tags](https://github.com/falcon/falcon-core/tags) are created
regularly from release branches to indicate new official, stable release versions of Falcon Core.

The contribution workflow is described in [CONTRIBUTING.md](CONTRIBUTING.md)
and useful hints for developers can be found in [doc/developer-notes.md](doc/developer-notes.md).

Testing
-------

Testing and code review is the bottleneck for development; we get more pull
requests than we can review and test on short notice. Please be patient and help out by testing
other people's pull requests, and remember this is a security-critical project where any mistake might cost people
lots of money.

### Automated Testing

Developers are strongly encouraged to write [unit tests](src/test/README.md) for new code, and to
submit new unit tests for old code. Unit tests can be compiled and run
(assuming they weren't disabled in configure) with: `make check`. Further details on running
and extending unit tests can be found in [/src/test/README.md](/src/test/README.md).

There are also [regression and integration tests](/test), written
in Python.
These tests can be run (if the [test dependencies](/test) are installed) with: `test/functional/test_runner.py`

The CI (Continuous Integration) systems make sure that every pull request is built for Windows, Linux, and macOS,
and that unit/sanity tests are run automatically.

### Manual Quality Assurance (QA) Testing

Changes should be tested by somebody other than the developer who wrote the
code. This is especially important for large or high-risk changes. It is useful
to add a test plan to the pull request description if testing the changes is
not straightforward.

