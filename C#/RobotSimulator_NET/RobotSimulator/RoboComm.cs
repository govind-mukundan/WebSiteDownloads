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
        // Asynchronous delegates using Begin/End Invoke - Obsolete
        public delegate bool asyncDataProcessHandler(byte[] array);
        public asyncDataProcessHandler asyncDataProcessDelegate;
        // Asynchronous delegate using TPL and await
        public Func<byte[], Task<bool>> asyncTplDataReceivedDelegate;

        public RoboComm()
        {
            RobotState = 0;
            StartCommThread(); // Start Communicating
        }

        async private void OnTimeElapsed(object source, ElapsedEventArgs e)
        {
            Console.WriteLine("ChildThread:The Elapsed event was raised at {0}", e.SignalTime);

            if (dataReceivedDelegate != null)
            {
                byte[] test = {1,2,3,4};
                dataReceivedDelegate(test);
            }

            // Call backs using TPL
            if (asyncTplDataReceivedDelegate != null)
            {
                byte[] test = { 1, 2, 3, 4 };
                Task<bool> task = Task.Run(() => 
                {
                    return (asyncTplDataReceivedDelegate(test));
                });

                // This callback will execute once the task completes
                task.ContinueWith(_ => 
                {
                    Console.WriteLine(" The result in TPL callback is: {0}.", task.Result);
                });
            }

            // Another way using TPL and await keyword
            if (asyncTplDataReceivedDelegate != null)
            {
                byte[] test = { 1, 2, 3, 4 };

                // Pass the delegate as a task to the system and await the result
                bool awaited_result = await asyncTplDataReceivedDelegate(test);
                // Control will come here only after the asyncTplDataReceivedDelegate returns
                Console.WriteLine("Awaited result = " + awaited_result.ToString());
            }

            // Obsolete way of being asynchronous using BeginInvoke/EndInvoke
            if (asyncDataProcessDelegate != null)
            {
                byte[] test = { 1, 2, 3, 4 };
                // Option 1 - Wait for completion method.
                IAsyncResult iar = asyncDataProcessDelegate.BeginInvoke(test, null, null); // Start async
                while (!iar.IsCompleted)
                {
                    Console.Write(".");
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


        #region TAP based solution
        async public Task<byte[]> ReadBytesFromRobotAsync()
        {
        TaskCompletionSource<byte[]> dataRxTcs = new TaskCompletionSource<byte[]>();
        Console.WriteLine("Awaiting begins ...");
        await Task.Delay(C_COMM_INTERVAL).ContinueWith(_ => { dataRxTcs.SetResult(new byte[] { 3, 4, 5, 6, 7 }); });
        Console.WriteLine("Awaiting ends ...");
        return dataRxTcs.Task.Result;
        }
        #endregion

    }
}
