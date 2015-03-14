using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.IO.Ports;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;

namespace SharpGLWPFPlot
{

    /// <summary>
    /// This class acts as an interface to a Windows Serial Port. It constantly reads data from the port and passes it to someone else who can interpret the data 
    /// via a delegate. The class that interprets data should be able to handle byte streams.
    /// </summary>
    class SerialPortAdapter
    {
        public bool Simulate { get { return _simulate; } set { _simulate = value; } }
        /// <summary>
        /// A delegate that takes the bytes received and tells you how many bytes have been processed as return
        /// </summary>
        public Func<byte[], int> SerialDataRxedHandler { get; set; }

        SerialPort _serialPort;
        int BT_BAUD_RATE = 115200;
        bool _keepReading = false;
        byte[] _dataBuffer;  // Buffer to store data received over COM port for analysis
        byte[] _payloadBuffer;  // Buffer to store current payload for analysis
        int _buffHead;
        int _buffTail;
        private const int BUFF_SIZE = 1024;
        byte[] C_START_CHAR = new byte[] { (byte)'Z', 0 };
        byte[] C_STOP_CHAR = new byte[] { (byte)'X', 0 };
        CancellationTokenSource _cts; // A cancellation token for Stream.ReadAsync
        bool _simulate = false;
        BinaryReader _sr;

        /// <summary>
        /// Open a serial port and start reading data (if available)
        /// </summary>
        public bool Open(string portName)
        {
            bool ret = false;

            if (InitSerialSettings(portName))
            {
                _dataBuffer = new byte[BUFF_SIZE];
                _buffHead = 0;
                _buffTail = 0;
                _payloadBuffer = new byte[BUFF_SIZE >> 1];
                //AccelerationBuffer = new AccelerationPoint[C_ACC_BUFF_SIZE];
                //AccBufferHead = 0;
                _keepReading = true;
                // Start the read thread
                Task task = Task.Factory.StartNew(() => ReadPortAsync());
                task.ContinueWith(c => { MessageBox.Show("Error running serial port read task..", "Oh No!", MessageBoxButton.OK, MessageBoxImage.Hand); }, TaskContinuationOptions.OnlyOnFaulted | TaskContinuationOptions.ExecuteSynchronously);
                ret = true;


            }
            return (ret);
        }

        /// <summary>
        /// Close serial port
        /// </summary>
        public void Close()
        {
            Debug.WriteLine("Closing Serial Port");
            if (_serialPort.IsOpen)
            {
                _cts.Cancel();
                _serialPort.Close();
                _keepReading = false;
                _buffHead = 0;
                _buffTail = 0;

            }


        }
        /// <summary>
        /// Custom command to start streaming
        /// </summary>
        public void StartStreaming()
        { 
            if (_simulate)
            {
                FileStream fs = new System.IO.FileStream("Simulation.pcm", System.IO.FileMode.Open);
                _sr = new BinaryReader(fs, Encoding.UTF8);
                _keepReading = true;
                Task task = Task.Factory.StartNew(() => SimulateData());
            }
            else
            {
                if (_serialPort == null) return;
                // Start receiving data
                _serialPort.Write(C_START_CHAR, 0, 1);
            }
        }

        /// <summary>
        /// Custom command to stop streaming
        /// </summary>
        public void StopStreaming()
        {
            if (_simulate == true && _sr != null)
            {
                _sr.Close();
                _sr.Dispose();
                _keepReading = false;
            }
            else
            {
                if (_serialPort == null) return;

                _serialPort.Write(C_STOP_CHAR, 0, 1);
            }

        }

        /// <summary>
        /// Initialize the serial port settings
        /// </summary>
        /// <param name="portName"></param>
        /// <returns></returns>
        private bool InitSerialSettings(string portName)
        {
            try
            {
                _serialPort = new SerialPort(portName, BT_BAUD_RATE, Parity.None, 8, StopBits.One);
                _serialPort.Handshake = Handshake.None;
                // _serialPort.DataReceived += new SerialDataReceivedEventHandler(sp_DataReceived);
                _serialPort.ReadTimeout = 1000;
                _serialPort.WriteTimeout = 1000;
                _serialPort.Open(); // handle unauthorized access exception !!
                _serialPort.ErrorReceived += (object self, SerialErrorReceivedEventArgs se_arg) => { Debug.Write("| Error: {0} | ", System.Enum.GetName(se_arg.EventType.GetType(), se_arg.EventType)); };
                _cts = new CancellationTokenSource();
                return true;
            }
            catch
            {
                MessageBox.Show("Error opening port, check settings or if port is already opened\nand try again", "Oh No!", MessageBoxButton.OK, MessageBoxImage.Hand);
                return false;
            }

        }

        /// <summary> Get the data and pass it on. </summary>
        private void ReadPort()
        {
            byte[] readBuffer = new byte[_serialPort.ReadBufferSize + 1];

            while (_keepReading)
            {
                if (_serialPort.IsOpen)
                {
                    try
                    {
                        // If there are bytes available on the serial port,
                        // Read returns up to "count" bytes, but will not block (wait)
                        // for the remaining bytes. If there are no bytes available
                        // on the serial port, Read will block until at least one byte
                        // is available on the port, up until the ReadTimeout milliseconds
                        // have elapsed, at which time a TimeoutException will be thrown.
                        int count = _serialPort.Read(readBuffer, 0, _serialPort.ReadBufferSize);
                        // Now that we have some data, dump it into the buffer and analyze it
                        BlockCopyToCircularBuffer(readBuffer, _dataBuffer, _buffHead & (BUFF_SIZE - 1), BUFF_SIZE, count);
                        if (count > 0)
                        {
                            if (SerialDataRxedHandler != null)
                            {
                                // 1. Copy the data into a temporary linear buffer
                                int len = (_buffHead - _buffTail) & (BUFF_SIZE - 1);
                                byte[] data = new byte[len];
                                BlockCopyFromCircularBuffer(_dataBuffer, _buffTail & (BUFF_SIZE - 1), BUFF_SIZE, data, len);
                                // 2. Send the data for further processing
                                _buffTail += SerialDataRxedHandler(data);
                            }
                        }
                    }
                    catch (TimeoutException) { }
                }
                else
                {
                    TimeSpan waitTime = new TimeSpan(0, 0, 0, 0, 50);
                    Thread.Sleep(waitTime);
                }
            }
        }

        /// <summary>
        /// Makes use of Buffer.BlockCopy when source buffer is a circular buffer
        /// </summary>
        void BlockCopyFromCircularBuffer(Array souceCircularBuffer, int startIndex, int size, Array destLinearBuffer, int count)
        {

            if (count + startIndex < size)
            {
                Buffer.BlockCopy(souceCircularBuffer, startIndex, destLinearBuffer, 0, count);
            }
            else // Need to wrap around the buffer
            {
                Buffer.BlockCopy(souceCircularBuffer, startIndex, destLinearBuffer, 0, size - startIndex);
                Buffer.BlockCopy(souceCircularBuffer, 0, destLinearBuffer, size - startIndex, count - (size - startIndex));
            }

        }

        /// <summary>
        /// Makes use of Buffer.BlockCopy when source buffer is a circular buffer
        /// </summary>
        void BlockCopyToCircularBuffer(Array souceLinearBuffer, Array destCircularBuffer, int startIndex, int size, int count)
        {

            if (count + startIndex < size)
            {
                Buffer.BlockCopy(souceLinearBuffer, 0, destCircularBuffer, startIndex, count);
            }
            else
            {
                Buffer.BlockCopy(souceLinearBuffer, 0, destCircularBuffer, startIndex, size - startIndex);
                Buffer.BlockCopy(souceLinearBuffer, size - startIndex, destCircularBuffer, 0, count - (size - startIndex));
            }
        }

        /// <summary>
        /// Thread to simulate data
        /// </summary>
        void SimulateData()
        {
            try
            {
                while (_keepReading)
                {
                    if (SerialDataRxedHandler != null && _sr != null)
                    {
                        byte[] data = new byte[8];
                        int count = _sr.Read(data, 0, 6);
                        SerialDataRxedHandler(data);

                        if (count == 0) // End of stream
                        {
                            _sr.Close();
                            _sr.Dispose();
                            FileStream fs = new System.IO.FileStream("Simulation.pcm", System.IO.FileMode.Open);
                            _sr = new BinaryReader(fs, Encoding.UTF8);
                        }
                    }
                    Thread.Sleep(5);
                }

                if (_sr != null)
                {
                    _sr.Close();
                    _sr.Dispose();
                }
            }
            catch (Exception ex) { Debug.Write(ex.Message); }
        }

        #region For NET 4.5 and above

        /// <summary>
        /// Makes use of ReadAsync of the serial ports base stream to read data
        /// </summary>
        async void ReadPortAsync()
        {

            byte[] readBuffer = new byte[_serialPort.ReadBufferSize];

            while (_keepReading)
            {
                if (_serialPort.IsOpen)
                {
                    try
                    {
                        // If there are bytes available on the serial port,
                        // Read returns up to "count" bytes, but will not block (wait)
                        // for the remaining bytes. If there are no bytes available
                        // on the serial port, Read will block until at least one byte
                        // is available on the port.
                        Task<int> t_count = _serialPort.BaseStream.ReadAsync(readBuffer, 0, readBuffer.Length, _cts.Token);
                        int count = await t_count;
                        // Now that we have some data, dump it into the buffer and analyze it
                        BlockCopyToCircularBuffer(readBuffer, _dataBuffer, _buffHead & (BUFF_SIZE - 1), BUFF_SIZE, count);
                        _buffHead += count;

                        if (count > 0)
                        {
                            if (SerialDataRxedHandler != null)
                            {
                                // 1. Copy the data into a temporary linear buffer
                                int len = (_buffHead - _buffTail) & (BUFF_SIZE - 1);
                                byte[] data = new byte[len];
                                BlockCopyFromCircularBuffer(_dataBuffer, _buffTail & (BUFF_SIZE - 1), BUFF_SIZE, data, len);
                                // 2. Send the data for further processing
                                _buffTail += SerialDataRxedHandler(data);

                            }
                        }
                    }
                    catch (Exception ex) { Debug.Write(ex.Message); }
                }
                else
                {
                    TimeSpan waitTime = new TimeSpan(0, 0, 0, 0, 50);
                    Thread.Sleep(waitTime);
                }
            }

        }

        #endregion

    }
}
