import { ContractAbi } from '@aztec/foundation/abi';
import { AztecAddress } from '@aztec/foundation/aztec-address';

import { Wallet } from '../aztec_rpc_client/wallet.js';
import { ContractBase } from './contract_base.js';

/**
 * The Contract class represents a contract and provides utility methods for interacting with it.
 * It enables the creation of ContractFunctionInteraction instances for each function in the contract's ABI,
 * allowing users to call or send transactions to these functions. Additionally, the Contract class can be used
 * to attach the contract instance to a deployed contract on-chain through the AztecRPCClient, which facilitates
 * interaction with Aztec's privacy protocol.
 */
export class Contract extends ContractBase {
  /**
   * Creates a contract instance.
   * @param address - The deployed contract's address.
   * @param abi - The Application Binary Interface for the contract.
   * @param wallet - The wallet to use when interacting with the contract.
   * @returns A promise that resolves to a new Contract instance.
   */
  public static async at(address: AztecAddress, abi: ContractAbi, wallet: Wallet): Promise<Contract> {
    const extendedContractData = await wallet.getExtendedContractData(address);
    if (extendedContractData === undefined) {
      throw new Error('Contract ' + address.toString() + ' is not deployed');
    }
    return new Contract(extendedContractData.getCompleteAddress(), abi, wallet);
  }
}
