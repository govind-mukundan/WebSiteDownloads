using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Timers;

namespace RobotSimulator
{
    public partial class Form1 : Form
    {
        private RoboComm Robot;
        private System.Timers.Timer RxTimer;
        private const int C_COMM_INTERVAL = 1000;


        public Form1()
        {
            InitializeComponent();
            listBox_Logger.Items.Add("Hello World");
            Robot = new RoboComm();
            // 1. Synchronous Delegates
            Robot.dataReceivedDelegate += ProcessDataFromBot;
            // Anonymous delegate version
            Robot.dataReceivedDelegate += delegate(byte[] data){
                this.Invoke((MethodInvoker)delegate
                {
                    listBox_Logger.Items.Add(DateTime.Now.ToString() + " And some Anon bytes");
                    listBox_Logger.SelectedIndex = listBox_Logger.Items.Count - 1;
                });
            };

            // 2. Asynchronous delegate using begin/end invoke (obsolete)
            Robot.asyncDataProcessDelegate += ProcessDataFromBotAsync;

            // 3. Asynchronous delegate using TPL and await keyword
            Robot.asyncTplDataReceivedDelegate = async (byte[] data) =>
            {
                this.Invoke((MethodInvoker)delegate
                {
                listBox_Logger_TPL.Items.Add(DateTime.Now.ToString() + " Some async/TPL bytes" );
                listBox_Logger_TPL.SelectedIndex = listBox_Logger_TPL.Items.Count - 1;
                });
                return (true);
            };

            StartParentThread();
        }

        private void ProcessDataFromBot(byte[] data)
        {
            this.Invoke((MethodInvoker)delegate
            {
                listBox_Logger.Items.Add(DateTime.Now.ToString() + " Got some bytes");
                listBox_Logger.SelectedIndex = listBox_Logger.Items.Count - 1;
            });

        }

        private bool ProcessDataFromBotAsync(byte[] data)
        {
            this.Invoke((MethodInvoker)delegate
            {
                listBox_Logger.Items.Add(DateTime.Now.ToString() + " Got Async bytes");
                listBox_Logger.SelectedIndex = listBox_Logger.Items.Count - 1;
            });
            return (true);
        }

        private void StartParentThread()
        {
            // Use a timer instead of a proper thread for simplicity
            RxTimer = new System.Timers.Timer(C_COMM_INTERVAL);
            RxTimer.Elapsed += new ElapsedEventHandler(OnTimeElapsed);
            RxTimer.Enabled = true;
        }

        private void OnTimeElapsed(object source, ElapsedEventArgs e)
        {
            Console.WriteLine("ParentThread: The Elapsed event was raised at {0}", e.SignalTime);

        }

        # region Task Based Asynchronous Pattern (TAP)
        // Here we write the code as if the entire communication operation were a synchronous activity
        // So we need a task that constantly polls the Robot and waits for data to be available

        async void ReadDataFromRobot()
        {
            // Get new data
            byte[] data = await Robot.ReadBytesFromRobotAsync();
            // Update the UI
            Console.WriteLine("Got data from Robot: {0}", data);
        }

        bool _isCancelled = false;
        async private void button1_Click(object sender, EventArgs e)
        {
            while (!_isCancelled)
            {
                // Get new data
                byte[] data = await Robot.ReadBytesFromRobotAsync();
                // Update the UI
                Console.WriteLine("Got data from Robot: " + ByteArrayToString(data));
                listBox_Logger_TAP.Items.Add(DateTime.Now.ToString() + " " + ByteArrayToString(data));
                listBox_Logger_TAP.SelectedIndex = listBox_Logger_TAP.Items.Count - 1;
            }
        }

        public static string ByteArrayToString(byte[] ba)
        {
            string hex = BitConverter.ToString(ba);
            return hex.Replace("-", "");
        }

        private void button2_Click(object sender, EventArgs e)
        {
            _isCancelled = true;
        }

        #endregion




    }
}
