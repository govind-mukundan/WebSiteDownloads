namespace RobotSimulator
{
    partial class Form1
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.listBox_Logger = new System.Windows.Forms.ListBox();
            this.listBox_Logger_TPL = new System.Windows.Forms.ListBox();
            this.button1 = new System.Windows.Forms.Button();
            this.button2 = new System.Windows.Forms.Button();
            this.listBox_Logger_TAP = new System.Windows.Forms.ListBox();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // listBox_Logger
            // 
            this.listBox_Logger.FormattingEnabled = true;
            this.listBox_Logger.Location = new System.Drawing.Point(12, 31);
            this.listBox_Logger.Name = "listBox_Logger";
            this.listBox_Logger.Size = new System.Drawing.Size(358, 212);
            this.listBox_Logger.TabIndex = 0;
            // 
            // listBox_Logger_TPL
            // 
            this.listBox_Logger_TPL.FormattingEnabled = true;
            this.listBox_Logger_TPL.Location = new System.Drawing.Point(616, 31);
            this.listBox_Logger_TPL.Name = "listBox_Logger_TPL";
            this.listBox_Logger_TPL.Size = new System.Drawing.Size(358, 212);
            this.listBox_Logger_TPL.TabIndex = 1;
            // 
            // button1
            // 
            this.button1.Location = new System.Drawing.Point(13, 269);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(131, 41);
            this.button1.TabIndex = 2;
            this.button1.Text = "Start TAP Robot";
            this.button1.UseVisualStyleBackColor = true;
            this.button1.Click += new System.EventHandler(this.button1_Click);
            // 
            // button2
            // 
            this.button2.Location = new System.Drawing.Point(13, 329);
            this.button2.Name = "button2";
            this.button2.Size = new System.Drawing.Size(131, 36);
            this.button2.TabIndex = 3;
            this.button2.Text = "Stop TAP Robot";
            this.button2.UseVisualStyleBackColor = true;
            this.button2.Click += new System.EventHandler(this.button2_Click);
            // 
            // listBox_Logger_TAP
            // 
            this.listBox_Logger_TAP.FormattingEnabled = true;
            this.listBox_Logger_TAP.Location = new System.Drawing.Point(201, 269);
            this.listBox_Logger_TAP.Name = "listBox_Logger_TAP";
            this.listBox_Logger_TAP.Size = new System.Drawing.Size(532, 186);
            this.listBox_Logger_TAP.TabIndex = 4;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(13, 12);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(209, 13);
            this.label1.TabIndex = 5;
            this.label1.Text = "Synchronous and Asynchronous delegates";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(613, 12);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(174, 13);
            this.label2.TabIndex = 6;
            this.label2.Text = "Asynchronous delegates using TPL";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(198, 253);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(265, 13);
            this.label3.TabIndex = 7;
            this.label3.Text = "Design using Task Based Asynchronous Patterh (TAP)";
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(986, 452);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.listBox_Logger_TAP);
            this.Controls.Add(this.button2);
            this.Controls.Add(this.button1);
            this.Controls.Add(this.listBox_Logger_TPL);
            this.Controls.Add(this.listBox_Logger);
            this.Name = "Form1";
            this.Text = "Form1";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ListBox listBox_Logger;
        private System.Windows.Forms.ListBox listBox_Logger_TPL;
        private System.Windows.Forms.Button button1;
        private System.Windows.Forms.Button button2;
        private System.Windows.Forms.ListBox listBox_Logger_TAP;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
    }
}

