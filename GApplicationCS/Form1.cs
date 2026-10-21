using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using god;

namespace god
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            StringBuilder b = new StringBuilder();
            CWinAPI.GetWindowText((int)this.Handle, b, 64);
            b.Replace('o', 'i');
            CWinAPI.SetWindowText((int)this.Handle, b);
        }

        private void button1_Click(object sender, EventArgs e)
        {
            if( button1.Enabled )
            {
                StringBuilder b = new StringBuilder();
                CWinAPI.GetWindowText((int)this.Handle, b, 64);
                b.Replace('i', 'o');
                CWinAPI.SetWindowText((int)this.Handle, b);

                button1.Enabled = false;
                button1.Hide();
                textBox1.Text = "Graphics initialized";

                int hWnd = (int)pictureBox1.Handle;
                GRuntime.create3DViewport(hWnd);
                textBox1.Text = "Graphics shutdown";
            }
        }

        private void pictureBox1_Click(object sender, EventArgs e)
        {

        }

        private void textBox1_TextChanged(object sender, EventArgs e)
        {
            if (textBox1.Text == "Graphics initialized")
            {
                textBox1.Text = "Graphics initialized";
                textBox1.SelectAll();
            }
        }

        private void button2_Click(object sender, EventArgs e)
        {

        }
    }
}
