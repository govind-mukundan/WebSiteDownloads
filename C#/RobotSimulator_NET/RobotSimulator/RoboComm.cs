using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Timers;
// To access the AsyncResult type
using System.Runtime.Remoting.Messaging;

namespace RobotSimulator
{
    class RoboComm
    {
        private Timer RoboCommTimer; // Simulate using a timer for simplicity
        private const int C_COMM_INTERVAL = 1000;
        private int RobotState;
        // Synchronous delegates
        public delegate void dataReceivedHandler(byte[] array);
        public dataReceivedHandler dataReceivedDelegate;
        // Asynchronous delegates
        public delegate bool asyncDataProcessHandler(byte[] array);
        public asyncDataProcessHandler asyncDataProcessDelegate;

        public RoboComm()
        {
            RobotState = 0;
            StartCommThread(); // Start Communicating
        }

        private void OnTimeElapsed(object source, ElapsedEventArgs e)
        {
            Console.WriteLine("ChildThread:The Elapsed event was raised at {0}", e.SignalTime);

            if (dataReceivedDelegate != null)
            {
                byte[] test = {1,2,3,4};
                dataReceivedDelegate(test);
            }

            if (asyncDataProcessDelegate != null)
            {
                byte[] test = { 1, 2, 3, 4 };
                // Option 1 - Wait for completion method.
                IAsyncResult iar = asyncDataProcessDelegate.BeginInvoke(test, null, null); // Start async
                while (!iar.IsCompleted)
                {
                    Console.WriteLine("Doing stuff");
                }
                bool result = asyncDataProcessDelegate.EndInvoke(iar); // Wait for end and get result
                Console.WriteLine("After EndInvoke: {0}", result);

                // Option 2 - use a callback, no waiting!!
                IAsyncResult iar2 = asyncDataProcessDelegate.BeginInvoke(test, new AsyncCallback(DelegateExOverCb), null); // Start async
                Console.WriteLine("Lets wait for the callback");

            }
        }

        void DelegateExOverCb(IAsyncResult iar)
        {
            // Get the deletate object from the iar object
            AsyncResult ar = (AsyncResult)iar;
            asyncDataProcessHandler del = (asyncDataProcessHandler)ar.AsyncDelegate;
            bool result = del.EndInvoke(iar);
            Console.WriteLine
               (" The result in callback is: {0}.", result);

        }

        private void StartCommThread()
        {
            // Use a timer instead of a proper thread for simplicity
            RoboCommTimer = new Timer(C_COMM_INTERVAL);
            RoboCommTimer.Elapsed += new ElapsedEventHandler(OnTimeElapsed);
            RoboCommTimer.Enabled = true;
        }
    }
}
