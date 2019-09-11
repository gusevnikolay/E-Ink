using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO.Ports;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace SimpleImage
{
    public partial class MainApp : Form
    {
        public MainApp()
        {
            InitializeComponent();
        }

        public byte[] GetBytesFromImage(Bitmap img)
        {
            var bmp = new Bitmap(img, 200, 200);
            var bits = new BitArray(200*200);
            for(int y = 0; y < bmp.Height; y++)
            {
                for(int x = 0; x < bmp.Width; x++)
                {
                    if (bmp.GetPixel(x, y).R > 127)
                    {
                        bits.Set(y*200+x, true);
                    }
                    else
                    {
                        bits.Set(y * 200 + x, false);
                    }
                }
            }
            byte[] data = new byte[5000];
            new BitArray(bits.Cast<bool>().Reverse().ToArray()).CopyTo(data,0);
            var reverse = new List<byte>();
            reverse.AddRange(data);
            reverse.Reverse();
            return reverse.ToArray();
        }   

        private void SendBytes(byte[] data)
        {
            var serial = new SerialPort("COM7", 115200);
            serial.Open();
            serial.Write(data, 0, data.Length);
            serial.Close();
        }

        private void button1_Click(object sender, EventArgs e)
        {
            var fileContent = string.Empty;
            var filePath = string.Empty;
            using (OpenFileDialog openFileDialog = new OpenFileDialog())
            {
                openFileDialog.RestoreDirectory = true;
                openFileDialog.Filter = "Image files (*.jpg, *.jpeg, *.jpe, *.jfif, *.png) | *.jpg; *.jpeg; *.jpe; *.jfif; *.png";
                if (openFileDialog.ShowDialog() == DialogResult.OK)
                {
                    filePath = openFileDialog.FileName;
                    Bitmap img = new Bitmap(Image.FromFile(filePath));
                    pictureBox1.Image = new Bitmap(img, pictureBox1.Width, pictureBox1.Height);
                    var bytes = GetBytesFromImage(img);
                    var list = new List<byte>();         
                    foreach (var b in bytes)
                    {
                        list.Add(b);
                        if (b == 0x55) list.Add(0);
                    }
                    list.Add(0x55);
                    list.Add(1);
                    SendBytes(list.ToArray());
                }
            }
        }
    }
}
