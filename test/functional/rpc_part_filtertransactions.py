#!/usr/bin/env python3
# Copyright (c) 2017-2021 The Particl Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

from test_framework.test_falcon import FalconTestFramework
from test_framework.authproxy import JSONRPCException


class FilterTransactionsTest(FalconTestFramework):
    def set_test_params(self):
        self.setup_clean_chain = True
        self.num_nodes = 3
        self.extra_args = [ [ '-debug', '-reservebalance=10000000' ] for i in range(self.num_nodes) ]

    def skip_test_if_missing_module(self):
        self.skip_if_no_wallet()

    def setup_network(self, split=False):
        self.add_nodes(self.num_nodes, extra_args=self.extra_args)
        self.start_nodes()

        self.connect_nodes_bi(0, 1)
        self.connect_nodes_bi(0, 2)
        self.connect_nodes_bi(1, 2)
        self.sync_all()

    def run_test(self):
        nodes = self.nodes

        # import keys for node wallets
        nodes[0].extkeyimportmaster('abandon baby cabbage dad eager fabric gadget habit ice kangaroo lab absorb')
        nodes[1].extkeyimportmaster('drip fog service village program equip minute dentist series hawk crop sphere olympic lazy garbage segment fox library good alley steak jazz force inmate')
        nodes[2].extkeyimportmaster('sección grito médula hecho pauta posada nueve ebrio bruto buceo baúl mitad')

        # create addresses
        selfAddress    = nodes[0].getnewaddress('self')
        selfAddress2   = nodes[0].getnewaddress('self2')
        selfStealth    = nodes[0].getnewstealthaddress('stealth')
        selfSpending   = nodes[0].getnewaddress('spending', 'false', 'false', 'true')
        targetAddress  = nodes[1].getnewaddress('target')
        targetStealth  = nodes[1].getnewstealthaddress('taret stealth')
        stakingAddress = nodes[2].getnewaddress('staking')

        # Simple PART transaction
        nodes[0].sendtoaddress(targetAddress, 10)
        self.stakeBlocks(1)

        txids = []
        txids.append(nodes[1].sendtoaddress(selfAddress, 8))


        # PART to BLIND
        txids.append(nodes[0].sendtypeto('part', 'blind', [{'address': selfStealth, 'amount': 20, 'narr': 'node0 -> node0 p->b'},]))

        # PART to ANON
        txids.append(nodes[0].sendtypeto('part', 'anon', [{'address': targetStealth, 'amount': 20, 'narr': 'node0 -> node1 p->a'},]))

        # Several outputs
        txids.append(nodes[0].sendtypeto(
            'part',               # type in
            'part',               # type out
            [                     # outputs
                {
                    'address':    selfAddress,
                    'amount':     1,
                    'narr':       'output 1'
                },
                {
                    'address':    selfAddress2,
                    'amount':     2,
                    'narr':       'output 2'
                }
            ]
        ))

        # cold staking: watchonly
        script = nodes[0].buildscript(
            {
                'recipe':        'ifcoinstake',
                'addrstake':     stakingAddress,
                'addrspend':     selfSpending
            }
        )
        txids.append(nodes[0].sendtypeto(
            'part',              # type in
            'part',              # type out
            [                    # outputs
                {
                    'address':   'script',
                    'amount':    200,
                    'script':    script['hex'],
                    'narr':      'activating cold staking'
                }
            ]
        ))
        txid = nodes[0].sendtoaddress(selfSpending, 50)
        txids.append(txid)
        txids.append(nodes[0].sendtypeto(
            'part',              # type in
            'part',              # type out
            [                    # outputs
                {
                    'address':   targetAddress,
                    'amount':    7.123,
                    'narr':      'watchonly transaction'
                }
            ],
            '',                  # comment
            '',                  # comment_to
            0,                   # ring size
            0,                   # inputs per sig
            False,               # test fee
            {                    # coincontrol
                'changeaddress': selfAddress,
                'inputs':        [{ 'tx': txid, 'n': 1 }],
                'replaceable':   False,
                'conf_target':   1,
                'estimate_mode': 'CONSERVATIVE'
            }
        ))

        for txid in txids:
            assert(self.wait_for_mempool(nodes[0], txid))
        self.stakeBlocks(1)

        # Check blinding factors
        fto = nodes[1].filtertransactions({'type': 'anon', 'show_blinding_factors': True})
        rtx = nodes[1].getrawtransaction(fto[0]['txid'], True, fto[0]['blockhash'])
        bfs = {}
        for vout in fto[0]['outputs']:
            if 'blindingfactor' in vout:
                bfs[vout['vout']] = (vout['amount'], vout['blindingfactor'])
        for vout in rtx['vout']:
            if vout.get('type', 'unknown') == 'anon':
                assert(nodes[1].verifycommitment(vout['valueCommitment'], bfs[vout['n']][1], bfs[vout['n']][0])['result'] is True)

        #
        # general
        #

        # Without argument
        assert(len(nodes[0].filtertransactions()) == 10)

        # Too many arguments
        try:
            nodes[0].filtertransactions('foo', 'bar')
            assert(False)
        except JSONRPCException as e:
            assert('filtertransactions' in e.error['message'])

        #
        # count
        #

        # count: -1 => JSONRPCException
        try:
            nodes[0].filtertransactions({ 'count': -1 })
            assert(False)
        except JSONRPCException as e:
            assert('Invalid count' in e.error['message'])

        # count: 0 => all transactions
        assert(len(nodes[0].filtertransactions({ 'count': 0 })) == 11)

        # count: 1
        assert(len(nodes[0].filtertransactions({ 'count': 1 })) == 1)

        #
        # skip
        #

        # skip: -1 => JSONRPCException
        try:
            nodes[0].filtertransactions({ 'skip': -1 })
            assert(False)
        except JSONRPCException as e:
            assert('Invalid skip' in e.error['message'])

        # skip = count => no entry
        ro = nodes[0].filtertransactions({ 'count': 50 })
        ro = nodes[0].filtertransactions({ 'skip': len(ro) })
        assert(len(ro) == 0)

        # skip == count - 1 => one entry
        ro = nodes[0].filtertransactions({ 'count': 50 })
        ro = nodes[0].filtertransactions({ 'skip': len(ro) - 1 })
        assert(len(ro) == 1)

        # skip: 1
        ro = nodes[0].filtertransactions({
            'category': 'send',
            'count':    20,
            'skip':     1
        })
        assert(float(ro[0]['amount']) == -20.0)

        #
        # include_watchonly
        #

        ro = nodes[2].filtertransactions({ 'include_watchonly': False })
        assert(len(ro) == 0)
        ro = nodes[2].filtertransactions({ 'include_watchonly': True })
        assert(len(ro) == 1)

        #
        # search
        #

        queries = [
            [targetAddress, 2],
            [selfStealth,   1],
            ['71230',       1]
        ]

        for query in queries:
            ro = nodes[0].filtertransactions({ 'search': query[0] })
            assert(len(ro) == query[1])

        #
        # category
        #

        # TODO
        # 'orphan' transactions
        # 'immature' transactions
        # 'orphaned_stake' transactions

        categories = [
            ['internal_transfer', 4],
            ['coinbase',          1],
            ['send',              3],
            ['receive',           1],
            ['stake',             0]
        ]

        for category in categories:
            ro = nodes[0].filtertransactions({ 'category': category[0] })
            for t in ro:
                assert(t['category'] == category[0])
            if (category[0] != 'stake'):
                assert(len(ro) == category[1])

        # category 'all'
        length = len(nodes[0].filtertransactions({'count': 20}))
        ro = nodes[0].filtertransactions({ 'category': 'all', 'count': 20 })
        assert(len(ro) == length)

        # invalid transaction category
        try:
            ro = nodes[0].filtertransactions({ 'category': 'invalid' })
            assert(False)
        except JSONRPCException as e:
            assert('Invalid category' in e.error['message'])

        #
        # type
        #

        # type 'all'
        length = len(nodes[0].filtertransactions({'count': 20}))
        ro = nodes[0].filtertransactions({ 'type': 'all', 'count': 20 })
        assert(len(ro) == length)

        # type 'standard'
        ro = nodes[0].filtertransactions({ 'type': 'standard', 'count': 20 })
        assert(len(ro) == 9)
        for t in ro:
            assert('type' not in t)
            for o in t['outputs']:
                assert('type' not in o)

        # type 'anon'
        ro = nodes[0].filtertransactions({ 'type': 'anon', 'count': 20 })
        assert(len(ro) == 1)
        for t in ro:
            foundA = False
            for o in t['outputs']:
                if 'type' in o and o['type'] == 'anon':
                    foundA = True
                    break
            assert(foundA is True)

        # type 'blind'
        ro = nodes[0].filtertransactions({ 'type': 'blind', 'count': 20 })
        assert(len(ro) == 1)
        for t in ro:
            foundB = False
            for o in t['outputs']:
                if 'type' in o and o['type'] == 'blind':
                    foundB = True
                    break
            assert(foundB is True)

        # invalid transaction type
        try:
            ro = nodes[0].filtertransactions({ 'type': 'invalid' })
            assert(False)
        except JSONRPCException as e:
            assert('Invalid type' in e.error['message'])

        #
        # sort
        #

        sortable_fields = [
            [ 'time',          'desc' ],
            [ 'address',        'asc' ],
            [ 'category',       'asc' ],
            [ 'amount',        'desc' ],
            [ 'confirmations', 'desc' ],
            [ 'txid',           'asc' ]
        ]

        for sorting in sortable_fields:
            ro = nodes[0].filtertransactions({ 'sort': sorting[0] })
            prev = None
            for t in ro:
                if "address" not in t and "stealth_address" in t:
                    t["address"] = t["stealth_address"]
                if "address" not in t and "stealth_address" in t["outputs"][0]:
                    t["address"] = t["outputs"][0]["stealth_address"]
                if "address" not in t and "address" in t["outputs"][0]:
                    t["address"] = t["outputs"][0]["address"]
                if t["amount"] < 0:
                    t["amount"] = -t["amount"]
                if prev is not None:
                    if sorting[1] == 'asc':
                        assert(t[sorting[0]] >= prev[sorting[0]])
                    if sorting[1] == 'desc':
                        assert(t[sorting[0]] <= prev[sorting[0]])
                prev = t

        # invalid sort
        try:
            ro = nodes[0].filtertransactions({ 'sort': 'invalid' })
            assert(False)
        except JSONRPCException as e:
            assert('Invalid sort' in e.error['message'])

        # Sent blind should show when filtered for blinded txns
        nodes[0].sendtypeto('blind', 'part', [{'address': targetStealth, 'amount': 1.0},])
        ro = nodes[0].filtertransactions({ 'type': 'blind', 'count': 20 })
        assert(len(ro) == 2)


if __name__ == '__main__':
    FilterTransactionsTest().main()
