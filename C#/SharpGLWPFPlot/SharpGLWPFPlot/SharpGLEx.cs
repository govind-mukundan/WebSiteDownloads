using GlmNet;
using SharpGL;
using SharpGL.VertexBuffers;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using DR = System.Drawing;

namespace SharpGLWPFPlot
{
    public static class SharpGLEx
    {
        /// <summary>
        /// Extension to enable the direct use of .NET Colors in Sharp GL
        /// </summary>
        /// <param name="gl"></param>
        /// <param name="color"></param>
        /// <returns></returns>
        public static vec3 Color(this OpenGL gl, DR.Color color)
        {
            return (new vec3(color.R, color.G, color.B));
        }

        /// <summary>
        /// Extension to enable use of Vec3[] arrays as input to the vertex buffer
        /// </summary>
        /// <param name="vb"></param>
        /// <param name="gl"></param>
        /// <param name="attributeIndex"></param>
        /// <param name="points"></param>
        /// <param name="isNormalised"></param>
        /// <param name="stride"></param>
        public static void SetData(this VertexBuffer vb, OpenGL gl, uint attributeIndex, vec3[] points, bool isNormalised, int stride)
        {
            float[] rawData = new float[points.Length * 3];
            for (int i = 0; i < points.Length; i++)
            {
                rawData[3 * i] = points[i].x;
                rawData[3 * i + 1] = points[i].y;
                rawData[3 * i + 2] = points[i].z;
            }

            vb.SetData(gl, attributeIndex, rawData, isNormalised, stride);
        }
        
    }
}
