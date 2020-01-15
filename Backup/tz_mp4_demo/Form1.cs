using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Runtime.InteropServices;
using System.Threading;

namespace tz_mp4_demo
{
    public partial class Form1 : Form


    {
        public Form1()
        {
            InitializeComponent();
        }

        [DllImport(@"..\..\..\x64\Debug\tz_mp4.dll")]
        extern static bool open_mp4(ref Int32 lPlayID,IntPtr sFilePath,Int32 w,Int32 h,Int32 frameRate);

        private void button1_Click(object sender, EventArgs e)
        {
           // bool ret = open_mp4();

        }
    }
}
