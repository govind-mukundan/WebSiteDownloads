using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace SharpGLWPFPlot
{
    class ECG
    {
        // LIS3DH data always ranges between these two values
        public static int C_MAX_LSB = 2000;
        public static int C_MIN_LSB = -2000;
        public static int C_ACC_RANGE_FACTOR = 1; // 1 for 2g, 2 for 4g, 3 for 8g and 4 for 16g

        public static int C_MIN_LSB_ECG = -32768;
        public static int C_MAX_LSB_ECG = 32767;

        int C_NUM_BYTES_PER_DATA_POINT = 2;

        BlockingCollection<int> AccQ = new BlockingCollection<int>(new ConcurrentQueue<int>());
        // Queue<AccelerationPoint> AccQueue = new Queue<AccelerationPoint>();
        // object QueueLock; // A lock to prevent enque and deque from messing each other up

        public int PointsAvailable
        {
            get
            {
                if (AccQ != null) return (AccQ.Count);
                else return 0;
            }
        }

        /// <summary>
        /// Returns an array of Acceleration points available.
        /// Just get the data from here and plot it!
        /// </summary>
        /// <returns></returns>
        public int[] GetAvailablePoints()
        {
            if (AccQ == null || AccQ.Count == 0)
                return (null);

            int items = AccQ.Count;
            int[] acc_array = new int[items];
            for (int i = 0; i < items; i++)
            {
                acc_array[i] = AccQ.Take(); // will block if it's not available
            }
            return (acc_array);
        }

        /// <summary>
        /// Takes in a stream of bytes and converts it into accelerometer data points which are stored in an internal queue.
        /// </summary>
        /// <param name="bytes"></param>
        /// <returns> Number of bytes successfully converted into valid data. If no bytes were read, 0 is returned.</returns>
        public int ECGByteStreamParser(byte[] bytes)
        {
            // An accelerometer data point contains at least 6 bytes
            // If there are less than 6 bytes, it's discarded
            int len = bytes.Length;
            len = len / C_NUM_BYTES_PER_DATA_POINT;

            for (int i = 0; i < len; i = i + C_NUM_BYTES_PER_DATA_POINT)
            {
                int ap = getInt16(bytes[i + 1], bytes[i + 0]);
                AccQ.Add(ap);
            }
            return (len * C_NUM_BYTES_PER_DATA_POINT);
        }

        /// <summary>
        /// Convert to signed integer from signed msb/lsb pair
        /// </summary>
        /// <param name="msb"></param>
        /// <param name="lsb"></param>
        /// <returns></returns>
        int getInt16(byte msb, byte lsb)
        {
            if ((msb & 0x80) == 0x80) //Negative number?
            {
                return (((int)(msb << 8) | (int)(lsb)) | (-1 << 16)); // Set all the MSB bits to 1 (except for the lower 16 bits)
            }
            else
            {
                return ((int)(msb << 8) | (int)(lsb));
            }
        }
    }
}
