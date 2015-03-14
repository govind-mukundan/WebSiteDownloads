using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using SharpGL.SceneGraph;
using SharpGL;
using SharpGL.Shaders;
using SharpGL.VertexBuffers;
using System.Diagnostics;
using GlmNet;
using System.Windows.Threading;
using DR = System.Drawing;

namespace SharpGLWPFPlot
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {

        //  The vertex buffer array which contains the vertex and colour buffers.
        VertexBufferArray vertexBufferArray;

        //  The shader program for our vertex and fragment shader.
        private ShaderProgram shaderProgram;

        uint attribute_vcol;
        uint attribute_vpos;
        uint uniform_mview;
        Accelerometer _accelInstance;
        SerialPortAdapter _serialPortAdapter;
        const float C_SAMPLES_PER_FRAME = 2000;

        vec3[] C_PLOT_COLORS = new vec3[] { new vec3(1f, 0f, 0f), new vec3(0f, 1f, 0f), new vec3(0f, 0f, 1f), new vec3(0f, 1f, 1f) };

        vec3[] xgrid;
        vec3[][] _data; // A buffer spanning the entire visible window to store vertices to be plotted. It's a jagged array with one row per sub-plot
        int[] _dataTail; // Pointer to last sample of data in the buffer. Only data until tail will be displayed
        vec3[][] _dataColor;
        int _index;
        static float C_X_MARGIN_MAX = 0.9999f;
        static float C_X_MARGIN_MIN = -0.9999f;
        static float C_Y_MARGIN_MAX = 0.9999f;
        static float C_Y_MARGIN_MIN = -0.9999f;
        static int C_NUM_Y_DIV = 3;
        const int C_SUB_PLOTS = 3;
        static float C_Y_STEP = (C_Y_MARGIN_MAX - C_Y_MARGIN_MIN) / C_NUM_Y_DIV;

        vec3[] vertdata;
        vec3[] coldata;
        mat4 mviewdata;

        /// <summary>
        /// Initializes a new instance of the <see cref="MainWindow"/> class.
        /// </summary>
        public MainWindow()
        {
            InitializeComponent();
        }

        /// <summary>
        /// Handles the OpenGLDraw event of the openGLControl1 control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="args">The <see cref="SharpGL.SceneGraph.OpenGLEventArgs"/> instance containing the event data.</param>
        private void openGLControl_OpenGLDraw(object sender, OpenGLEventArgs args)
        {
            //  Get the OpenGL object.
            OpenGL gl = openGLControl.OpenGL;

            if (_accelInstance == null) return;
            // If there's new data available then plot it
            AccelerationPoint[] points = _accelInstance.GetAvailablePoints();
            if (points != null)
            {
                // Extract Data from each of the axes and pass it to the drawing buffer
                float[] dataX = new float[points.Length];
                float[] dataY = new float[points.Length];
                float[] dataZ = new float[points.Length];
                for (int i = 0; i < points.Length; i++)
                {
                    dataX[i] = points[i].aX;
                    dataY[i] = points[i].aY;
                    dataZ[i] = points[i].aZ;
                }
                gl.Viewport(0, 0, (int)openGLControl.Width, (int)openGLControl.Height); // Set up the view port, which is the size of the control here.
                //  Clear the color and depth buffer.
                gl.Clear(OpenGL.GL_COLOR_BUFFER_BIT | OpenGL.GL_DEPTH_BUFFER_BIT | OpenGL.GL_STENCIL_BUFFER_BIT);

                //  Load the identity matrix.
                gl.LoadIdentity();
                // Redraw the entire viewport, including background that does not change
                CreateAndDrawGrid(gl);
                CreateAndPlotData(gl, dataX, 0);
                CreateAndPlotData(gl, dataY, 1);
                CreateAndPlotData(gl, dataZ, 2);
                openGLControl.InvalidateVisual(); // Force a re-draw of the control

                _index += points.Length;
                if (_index >= C_SAMPLES_PER_FRAME) _index = 0;
            }
        }

        /// <summary>
        /// Handles the OpenGLInitialized event of the openGLControl1 control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="args">The <see cref="SharpGL.SceneGraph.OpenGLEventArgs"/> instance containing the event data.</param>
        private void openGLControl_OpenGLInitialized(object sender, OpenGLEventArgs args)
        {
            //  TODO: Initialise OpenGL here.
            //  Get the OpenGL object.
            OpenGL gl = openGLControl.OpenGL;
            gl.Clear(OpenGL.GL_COLOR_BUFFER_BIT | OpenGL.GL_DEPTH_BUFFER_BIT | OpenGL.GL_STENCIL_BUFFER_BIT);
            //  Set a blue clear colour.
            gl.ClearColor(0.4f, 0.6f, 0.9f, 0.0f);
            gl.ClearColor(0f, 0f, 0f, 0.0f);
            //  Create the shader program.
            var vertexShaderSource = ManifestResourceLoader.LoadTextFile("vertex_shader.glsl");
            var fragmentShaderSource = ManifestResourceLoader.LoadTextFile("fragment_shader.glsl");
            shaderProgram = new ShaderProgram();
            shaderProgram.Create(gl, vertexShaderSource, fragmentShaderSource, null);
            attribute_vpos = (uint)gl.GetAttribLocation(shaderProgram.ShaderProgramObject, "vPosition");
            attribute_vcol = (uint)gl.GetAttribLocation(shaderProgram.ShaderProgramObject, "vColor");
            shaderProgram.AssertValid(gl);
            InitializeFixedBufferContents();
        }

        void CreateAndDrawGrid(OpenGL GL)
        {
            // Debug.WriteLine("Painting Begins..");

            // Create vertices for 4 lines that will split the figure into 3 equal sections
            xgrid = new vec3[(C_NUM_Y_DIV + 1) * 2];
            for (int i = 0; i < (C_NUM_Y_DIV + 1) * 2; i = i + 2)
            {
                xgrid[i].x = -1f; xgrid[i + 1].x = 1f;
                xgrid[i].y = C_X_MARGIN_MIN + C_Y_STEP * i / 2; xgrid[i + 1].y = C_X_MARGIN_MIN + C_Y_STEP * i / 2;
                xgrid[i].z = 0f; xgrid[i + 1].z = 0f;
            }

            coldata = new vec3[] { 
                GL.Color(DR.Color.White),
                GL.Color(DR.Color.White),
                GL.Color(DR.Color.White),
                GL.Color(DR.Color.White),
                GL.Color(DR.Color.White),
                GL.Color(DR.Color.White),
                GL.Color(DR.Color.White),
                GL.Color(DR.Color.White),
                GL.Color(DR.Color.White),
                GL.Color(DR.Color.White),
                GL.Color(DR.Color.White),
                GL.Color(DR.Color.White),
                GL.Color(DR.Color.White),
                GL.Color(DR.Color.White),
                GL.Color(DR.Color.White),
                GL.Color(DR.Color.White)
            };

            mviewdata = new mat4(1.0f);


            // --------------- Implementation WITH VERTEX ARRAY OBJECTS --------------------
            ////  Create the vertex array object.
            vertexBufferArray = new VertexBufferArray();
            vertexBufferArray.Create(GL);
            vertexBufferArray.Bind(GL);

            //  Create a vertex buffer for the vertex data.
            var vertexDataBuffer = new VertexBuffer();
            vertexDataBuffer.Create(GL);
            vertexDataBuffer.Bind(GL);
            vertexDataBuffer.SetData(GL, attribute_vpos, xgrid, false, 3);

            //  Now do the same for the colour data.
            var colourDataBuffer = new VertexBuffer();
            colourDataBuffer.Create(GL);
            colourDataBuffer.Bind(GL);
            colourDataBuffer.SetData(GL, attribute_vcol, coldata, false, 3);

            //  Unbind the vertex array, we've finished specifying data for it.
            vertexBufferArray.Unbind(GL);

            //  Bind the shader, set the matrices.
            shaderProgram.Bind(GL);
            //shaderProgram.SetUniformMatrix4(gl, "projectionMatrix", projectionMatrix.to_array());
            //shaderProgram.SetUniformMatrix4(gl, "viewMatrix", viewMatrix.to_array());
            shaderProgram.SetUniformMatrix4(GL, "modelview", mviewdata.to_array());

            //  Bind the out vertex array.
            vertexBufferArray.Bind(GL);

            GL.DrawArrays(OpenGL.GL_LINES, 0, 8);

            //  Unbind our vertex array and shader.
            vertexBufferArray.Unbind(GL);
            shaderProgram.Unbind(GL);

            // --------------- Implementation WITH VERTEX ARRAY OBJECTS END --------------------

        }

        void CreateAndPlotData(OpenGL GL, float[] raw_data, int figureID)
        {
            if (_dataTail[figureID] + raw_data.Length >= C_SAMPLES_PER_FRAME)
            {
                _dataTail[figureID] = 0; // Clear the buffer
            }
            AddVerticesToFigure(figureID, raw_data, _dataTail[figureID]);


            //  Create the vertex array object.
            vertexBufferArray = new VertexBufferArray();
            vertexBufferArray.Create(GL);
            vertexBufferArray.Bind(GL);

            //  Create a vertex buffer for the vertex data.
            var vertexDataBuffer = new VertexBuffer();
            vertexDataBuffer.Create(GL);
            vertexDataBuffer.Bind(GL);
            vertexDataBuffer.SetData(GL, attribute_vpos, _data[figureID], false, 3);

            //  Now do the same for the colour data.
            var colourDataBuffer = new VertexBuffer();
            colourDataBuffer.Create(GL);
            colourDataBuffer.Bind(GL);
            colourDataBuffer.SetData(GL, attribute_vcol, _dataColor[figureID], false, 3);

            //  Unbind the vertex array, we've finished specifying data for it.
            vertexBufferArray.Unbind(GL);

            //  Bind the shader, set the matrices.
            shaderProgram.Bind(GL);
            //shaderProgram.SetUniformMatrix4(gl, "projectionMatrix", projectionMatrix.to_array());
            //shaderProgram.SetUniformMatrix4(gl, "viewMatrix", viewMatrix.to_array());
            shaderProgram.SetUniformMatrix4(GL, "modelMatrix", mviewdata.to_array());

            //  Bind the out vertex array.
            vertexBufferArray.Bind(GL);

            //  Draw the square.
            GL.DrawArrays(OpenGL.GL_LINE_STRIP, 0, _dataTail[figureID]);

            //  Unbind our vertex array and shader.
            vertexBufferArray.Unbind(GL);
            shaderProgram.Unbind(GL);

        }


        /// <summary>
        /// Add a given set of raw data points onto the display buffer and update the tail pointers
        /// </summary>
        /// <param name="figNum"></param>
        /// <param name="point"></param>
        /// <param name="offset"></param>
        void AddVerticesToFigure(int figNum, float[] point, int offset)
        {
            float factor = 0;
            if (C_NUM_Y_DIV % 2 == 0)
                factor = 0.5f;

            for (int i = 0; i < point.Length; i++)
            {

               // 1. Normalize the point.x to range [-.33,.33] so we can fit in 3 figures in the range of [1,-1]
               _data[figNum][offset + i].y = 2 * (point[i] - Accelerometer.C_MIN_LSB) / (Accelerometer.C_MAX_LSB - Accelerometer.C_MIN_LSB) - 1; // [1,-1]

                _data[figNum][offset + i].y = _data[figNum][offset + i].y * C_Y_STEP / 2; // [-.33,.33]

                if (figNum == 0)
                    _data[figNum][offset + i].y = _data[figNum][offset + i].y + C_Y_STEP + factor * C_Y_STEP;
                else if (figNum == 1)
                    _data[figNum][offset + i].y += factor * C_Y_STEP;
                else if (figNum == 2)
                    _data[figNum][offset + i].y = _data[figNum][offset + i].y - C_Y_STEP - factor * C_Y_STEP;
            }

            _dataTail[figNum] += point.Length;
        }

        /// <summary>
        /// Since the X cordinates are fixed for each sample in the buffer and just depends on the actual size of the buffer, they can be initialized once and for all.
        /// </summary>
        void InitializeFixedBufferContents()
        {
            // Allocate memory
            _dataTail = new int[C_SUB_PLOTS];
            _data = new vec3[C_SUB_PLOTS][]; // Vertices
            _dataColor = new vec3[C_SUB_PLOTS][]; // Color for each vertex

            for (int j = 0; j < C_SUB_PLOTS; j++)
            {
                _data[j] = new vec3[(int)C_SAMPLES_PER_FRAME];
                _dataColor[j] = new vec3[(int)C_SAMPLES_PER_FRAME];

                for (int i = 0; i < C_SAMPLES_PER_FRAME; i++)
                {
                    _data[j][i].x = 2 * (i - 0) / (C_SAMPLES_PER_FRAME - 0) - 1;

                    // Set the color for each vertex
                    _dataColor[j][i] = C_PLOT_COLORS[j];
                }
            }
        }





        /// <summary>
        /// Handles the Resized event of the openGLControl1 control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="args">The <see cref="SharpGL.SceneGraph.OpenGLEventArgs"/> instance containing the event data.</param>
        private void openGLControl_Resized(object sender, OpenGLEventArgs args)
        {
            //  TODO: Set the projection matrix here.

            //  Get the OpenGL object.
            OpenGL gl = openGLControl.OpenGL;

            //  Set the projection matrix.
            gl.MatrixMode(OpenGL.GL_PROJECTION);

            //  Load the identity.
            gl.LoadIdentity();

            //  Create a perspective transformation.
            gl.Perspective(60.0f, (double)Width / (double)Height, 0.01, 100.0);

            //  Use the 'look at' helper function to position and aim the camera.
            gl.LookAt(-5, 5, -5, 0, 0, 0, 0, 1, 0);

            //  Set the modelview matrix.
            gl.MatrixMode(OpenGL.GL_MODELVIEW);
        }


        private void btnConnect_Click(object sender, RoutedEventArgs e)
        {
            _accelInstance = new Accelerometer();
            _serialPortAdapter = new SerialPortAdapter();
            _serialPortAdapter.SerialDataRxedHandler = _accelInstance.AccelerometerByteStreamParser;
            if (_serialPortAdapter.Open(cmbxPorts.Text))
            {
                btnStartStop.IsEnabled = true;
                Debug.WriteLine("Opening port to JIG:" + cmbxPorts.Text);
                btnConnect.IsEnabled = false;
            }

        }


        bool _enableStop = false;
        private void btnStart_Click(object sender, RoutedEventArgs e)
        {
            if (_enableStop == false)
            {
                _enableStop = true;
                btnStartStop.Content = "Stop";
                _serialPortAdapter.StartStreaming();
            }
            else
            {
                _enableStop = false;
                btnStartStop.Content = "Start";
                if (_accelInstance != null)
                {
                    _serialPortAdapter.StopStreaming();
                    _accelInstance.StopRecordingData();
                }
            }

        }


        private void Window_Closed(object sender, EventArgs e)
        {
            Debug.WriteLine("Application EXIT Triggered..");
            if (_serialPortAdapter != null)
            {
                _serialPortAdapter.Close();
            }
            System.Environment.Exit(0);
        }

        private void cbSimulate_Click(object sender, RoutedEventArgs e)
        {
            if (cbSimulate.IsChecked == true)
            {
                _accelInstance = new Accelerometer();
                _serialPortAdapter = new SerialPortAdapter();
                _serialPortAdapter.SerialDataRxedHandler = _accelInstance.AccelerometerByteStreamParser;
                _serialPortAdapter.Simulate = true;
                btnStartStop.IsEnabled = true;
                Debug.WriteLine("Entered Simulation Mode!!");
                btnConnect.IsEnabled = false;
            }
            else
            {
                if (_serialPortAdapter != null) { _serialPortAdapter.Simulate = false; }
                else
                {
                    btnStartStop.IsEnabled = false;
                    btnConnect.IsEnabled = true;
                }
                Debug.WriteLine("Exit Simulation Mode!!");
            }
        }

    }
}
