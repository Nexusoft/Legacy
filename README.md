Welcome to Nexus Core

You'll find what you need through the various file systems.
A good chunk of the Core Features are located in src/core.

I will spend more time updating the documentation for this code base as I continue to make progress developing the Trust System.

Updates Coming in Full 0.2.1 Release will include:\n
1. Trust Key Penalties to deprecate Trust Key Expiration.
2. Possible Trust Key Reactivation.
3. Push Packets in Mining LLP.
4. Mining LLP Version 1.1 Update from Pool Server
5. Possible Sequencing of UTXO into larger outputs every X blocks for Developer and Recycler Addresses.

Updates Coming in Full 0.2.2 Release will include:
1. LLP Messaging Integration for Better Network Communication
2. More Messaging Commands due to LLP integration allowing Core level Functions to Exist in greater efficiency.
3. Checkpoint Voting for Pending Checkpoint. 
4. Auto - re-forking besides just Reorganizations. Detect when your node is on a fork and automatically resolve.
5. Consensus algorithms by voting with trust key.

Updates Coming in Full 0.3.0 Release will include:
1. LLD database integration.
2. Light Blocks due to Memory Pool Synchronization Protocol
3. New protocols for Transaction Processing vs. Block Processing which means breaking Messaging Protocol to more Specialized Systems so Transactions and Blocks and Voting operate on Separate Protocols.
4. With lighter blocks, it won't be necessary to include full UTXO transaction information in the block, but rather the transaction hash that can verify the Merkle Root while blocks are validated on the block level upon receive, where the Transactions are verified in a separate call and answers on the validity of the block can be seen through the Trust Protocol. This will speed up block propagation through the network.
5. Integrating Miners into Trust Keys giving miners incentive to continue contribution to the Network. Miners will be penalized for less trust which means rewards will be less until full trust is established.

Contact me at colin@nexusoft.io with any suggestions / bugs. If you would like to help contribute contact me as well and we can speak about where you could fit into development.