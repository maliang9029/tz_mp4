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
using System.IO;


namespace tz_mp4_demo
{
    public partial class Form1 : Form


    {
        public Form1()
        {
            InitializeComponent();
        }

        [DllImport(@"D:\workplace\tz_mp4\x64\Debug\tz_mp4.dll")]
        extern static bool open_mp4(ref Int32 lPlayID,string sFilePath,Int32 w,Int32 h,Int32 frameRate);
        [DllImport(@"D:\workplace\tz_mp4\x64\Debug\tz_mp4.dll")]
        extern static bool write_frame(Int32 lPlayID, string sData, Int32 nDateLen);
        private void button1_Click(object sender, EventArgs e)
        {
            Int32 lPlayID = -1;
            bool ret = open_mp4(ref lPlayID, "D:\\workplace\\tz_mp4\\", 1920, 1080, 25);
		    if (!ret) {
		        return;
		    }
            StreamReader sr = new StreamReader("D:\\workplace\\tz_mp4\\linyuner.265");
		    char[] buffer = null;
		    while (sr.Peek() >= 0) {
			    buffer = new char[2048];
				int len = sr.Read(buffer, 0, buffer.Length);
                string str = new string(buffer);
                ret = write_frame(lPlayID, str, str.Length);
		   }
        }
    }
}
