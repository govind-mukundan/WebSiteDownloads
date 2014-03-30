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
                    listBox_Logger.Items.Add(DateTime.Now.ToString() + "And some Anon bytes");
                    listBox_Logger.SelectedIndex = listBox_Logger.Items.Count - 1;
                });
            };

            // 2. Asynchronous delegate
            Robot.asyncDataProcessDelegate += ProcessDataFromBotAsync;

            StartParentThread();
        }

        private void ProcessDataFromBot(byte[] data)
        {
            this.Invoke((MethodInvoker)delegate
            {
                listBox_Logger.Items.Add(DateTime.Now.ToString() + "Got some bytes");
                listBox_Logger.SelectedIndex = listBox_Logger.Items.Count - 1;
            });

        }

        private bool ProcessDataFromBotAsync(byte[] data)
        {
            this.Invoke((MethodInvoker)delegate
            {
                listBox_Logger.Items.Add(DateTime.Now.ToString() + "Got Async bytes");
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
    }
}
