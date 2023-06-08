import { BufferReader, serializeBufferToVector } from '@aztec/foundation/serialize';
import { randomBytes } from 'crypto';

/**
 * Data container of unverified data corresponding to one L2 block.
 */
export class NoirLogs {
  constructor(
    /**
     * Chunks of unverified data corresponding to individual pieces of information (e.g. Encrypted preimages).
     */
    public readonly dataChunks: Buffer[],
  ) {}

  /**
   * Serializes unverified data into a buffer.
   * @returns A buffer containing the serialized unverified data.
   */
  public toBuffer(): Buffer {
    // Serialize each buffer into the new buffer with prefix
    const serializedChunks = this.dataChunks.map(buffer => serializeBufferToVector(buffer));
    // Concatenate all serialized chunks into a single buffer
    const serializedBuffer = Buffer.concat(serializedChunks);

    return serializedBuffer;
  }

  /**
   * Get the total length of all data chunks in the instance if the data was serialized.
   * @returns Total length of data chunks.
   */
  public getSerializedLength(): number {
    // Adding 4 to each chunk's length to account for the size stored in the serialized buffer.
    return this.dataChunks.reduce((acc, chunk) => acc + chunk.length + 4, 0);
  }

  /**
   * Creates a new EventLogs object by concatenating multiple ones.
   * @param datas - The individual data objects to concatenate.
   * @returns A new EventLogs object whose chunks are the concatenation of the chunks.
   */
  public static join(datas: NoirLogs[]): NoirLogs {
    return new NoirLogs(datas.flatMap(chunk => chunk.dataChunks));
  }

  /**
   * Deserializes unverified data from a buffer.
   * @param buf - The buffer containing the serialized unverified data.
   * @returns A new EventLogs object.
   */
  public static fromBuffer(buf: Buffer | BufferReader): NoirLogs {
    const reader = BufferReader.asReader(buf);

    const chunks = reader.readBufferArray();
    return new NoirLogs(chunks);
  }

  /**
   * Creates a new EventLogs object with `numChunks` random data.
   * @param numChunks - The number of chunks to create.
   * @returns A new EventLogs object.
   */
  public static random(numChunks: number): NoirLogs {
    const chunks: Buffer[] = [];
    for (let i = 0; i < numChunks; i++) {
      chunks.push(randomBytes(144));
    }
    return new NoirLogs(chunks);
  }
}
