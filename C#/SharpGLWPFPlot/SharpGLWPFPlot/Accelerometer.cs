using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.IO.Ports;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Forms;

namespace SharpGLWPFPlot
{
    public class AccelerationPoint
    {
        public float aX;
        public float aY;
        public float aZ;
    }

    class Accelerometer
    {
        // LIS3DH data always ranges between these two values
        public static int C_MAX_LSB = 2000;
        public static int C_MIN_LSB = -2000;
        public static int C_ACC_RANGE_FACTOR = 1; // 1 for 2g, 2 for 4g, 3 for 8g and 4 for 16g

        int C_NUM_BYTES_PER_DATA_POINT = 6;

        BlockingCollection<AccelerationPoint> AccQ = new BlockingCollection<AccelerationPoint>(new ConcurrentQueue<AccelerationPoint>());
       // Queue<AccelerationPoint> AccQueue = new Queue<AccelerationPoint>();
       // object QueueLock; // A lock to prevent enque and deque from messing each other up
        BinaryWriter _sw;

        public void StartRecordingData()
        {
            FileStream fs = new System.IO.FileStream("StreamedACC.pcm", System.IO.FileMode.Create);
            _sw = new BinaryWriter(fs, Encoding.UTF8);
        }

        public void StopRecordingData()
        {
            if (_sw == null) return;

            _sw.Close();
            _sw.Dispose();
        }

        public int PointsAvailable
        {
            get {
                if (AccQ != null) return (AccQ.Count);
                else return 0; }
        }

        /// <summary>
        /// Returns an array of Acceleration points available.
        /// Just get the data from here and plot it!
        /// </summary>
        /// <returns></returns>
        public AccelerationPoint[] GetAvailablePoints()
        {
            if (AccQ == null || AccQ.Count == 0)
                return (null);

            int items = AccQ.Count;
            AccelerationPoint[] acc_array = new AccelerationPoint[items];
            for(int i = 0; i < items; i++){
                acc_array[i] = AccQ.Take(); // will block if it's not available
            }
            return (acc_array);
        }

     //   int _byteIndex; 
        /// <summary>
        /// Takes in a stream of bytes and converts it into accelerometer data points which are stored in an internal queue.
        /// </summary>
        /// <param name="bytes"></param>
        /// <returns> Number of bytes successfully converted into valid data. If no bytes were read, 0 is returned.</returns>
        public int AccelerometerByteStreamParser(byte[] bytes)
        {
            // An accelerometer data point contains at least 6 bytes
            // If there are less than 6 bytes, it's discarded
            int len = bytes.Length;
            len = len / C_NUM_BYTES_PER_DATA_POINT;

            for (int i = 0; i < len; i++)
            {
                int offset = i * C_NUM_BYTES_PER_DATA_POINT;
                AccelerationPoint ap = new AccelerationPoint();
                ap.aX = getInt16(bytes[offset + 1], bytes[offset + 0]);
                ap.aY = getInt16(bytes[offset + 3], bytes[offset + 2]);
                ap.aZ = getInt16(bytes[offset + 5], bytes[offset + 4]);
                if (_sw != null)
                {
                    for (int j = 0; j < C_NUM_BYTES_PER_DATA_POINT; j++)
                    {
                        _sw.Write(bytes[offset + j]);
                    }
                        
                }
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
