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
            this.SuspendLayout();
            // 
            // listBox_Logger
            // 
            this.listBox_Logger.FormattingEnabled = true;
            this.listBox_Logger.Location = new System.Drawing.Point(12, 12);
            this.listBox_Logger.Name = "listBox_Logger";
            this.listBox_Logger.Size = new System.Drawing.Size(358, 212);
            this.listBox_Logger.TabIndex = 0;
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(382, 269);
            this.Controls.Add(this.listBox_Logger);
            this.Name = "Form1";
            this.Text = "Form1";
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.ListBox listBox_Logger;
    }
}

