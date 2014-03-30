function serialplot()
%%%%%%%%%%%%%%%%%%%%%%%%
% This function plots data received from the serial port in real time, kind
% of like an oscilloscope. Data is read from the input buffer every second 
% and plotted. Plotting each sample individually is too slow for a high
% sampling rate (like 4ms or so). Once a complete page is displayed, the
% data is cleared and plot is reset.
% The Y axes is adjusted dynamically, disable it if you want.
% To stop the script cilck on the plot figure, the script will stop at the
% end of the current page. You can also initialize Total_Page_Limit to some
% other value if you only want say 10 pages to be plotted.
% Timeout Warning: If data was not received via the serial port, matlab
% issues a timeout warning, you can exit on this too.
%   Author: Govind Mukundan.
%   Copyright: You are free to do whatever the heck you want to do with
%              this file, except kill me.
%   Revision: 1.0 $  $Date: 2012/08/21
%%%%%%%%%%%%%%%%%%%%%%%%

%run('clean');
clear all;
close all;
newobjs = instrfind ; 
if(~isempty(newobjs))
fclose(newobjs); % closes all open serial ports incease they were not closed properly before.
end

%%%%%%%%%%%%%%%%%%%%%%%%
% Plot/Data Settings %
%%%%%%%%%%%%%%%%%%%%%%%%
% sampling rate = 4ms (250 sps), display 5s per page
Fs = 250;
Seconds_Per_Page = 5;
X_Limit = Fs * Seconds_Per_Page;
Total_Page_Limit = 100;
% Range of values from 0 to 50
Y_Limit_High = 50;
Y_Limit_Low = 0;
x = 0;
figure('WindowButtonDownFcn',@myCallback,'Units','normalized','Position',[0 0 1 1]); %set callback for stopping image
%figure('Units','normalized','Position',[0 0 1 1]); % Maximizes the figure
hLine = plot(nan,'-o');       %# Initialize a plot line (which isn't displayed yet because the values are NaN)
axis auto; 
xlim([0,X_Limit]);  % static limits are better than auto see: http://undocumentedmatlab.com/blog/plot-performance/
% ip data is of the format : +ABCD-+FFFF-+.... i.e. 6 characters per
% sample.
Ip_Data_Size = 6;

%%%%%%%%%%%%%%%%%%%%%%%%
% Serial Port Settings %
%%%%%%%%%%%%%%%%%%%%%%%%
s = serial('COM31'); %assigns the object s to serial port 
set(s, 'InputBufferSize', Fs*Ip_Data_Size); %number of bytes in input buffer
set(s, 'FlowControl', 'hardware');
set(s, 'BaudRate', 115200);
set(s, 'Parity', 'none');
set(s, 'DataBits', 8);
set(s, 'StopBit', 1);
set(s, 'Timeout',10); 
disp(get(s,'Name'));
prop(1)=(get(s,'BaudRate'));
prop(2)=(get(s,'DataBits'));
prop(3)=(get(s, 'StopBit'));
prop(4)=(get(s, 'InputBufferSize')); 
disp(['Port Setup Done!!',num2str(prop)]);
fopen(s);           %opens the serial port

%%%%%%%%%%%%%%%%%%%%%%%%
% Filter Configuration %
%%%%%%%%%%%%%%%%%%%%%%%%
[b_lp,a_lp] = butter(8,40/(Fs/2),'low');
global dbuffer ;
dbuffer = zeros(length(b_lp),1); % buffer for LPF
global N;
N = 2;
global Dbuffer ; % buffer for moving avg filter
Dbuffer = zeros(N,1);
% Second order notch
%wo = 50/(250/2);  bw = wo/25;
%[b_n,a_n] = iirnotch(wo,bw);
 fo = 50;  q = 25; bw = (fo/(Fs/2))/q;
[b_n,a_n] = iircomb(6,bw,'notch'); % Note type flag 'notch'
global nbuffer ;
nbuffer = zeros(length(b_n),1); % buffer for Notch
%%%%%%%%%%%%%%%%%%%%%%%%
% Init Variables %
%%%%%%%%%%%%%%%%%%%%%%%%
n=0;
num = zeros(1,Fs);
t = 0;


disp('Running');
while(n < Total_Page_Limit)
        clf; % clear current figure (refresh the polt)
        xlim([0,X_Limit]); ylim([Y_Limit_Low,Y_Limit_High]);  % static limits
        grid on;
        hold on;
        disp('refresh');
    while(t < X_Limit)  %Runs for 200 cycles - if you cant see the symbol, it is "less than" sign. so while (t less than 200)
        % read 1s worth of samples = 250 * 6
        index = 1;
        num_index = 1;
        % Handle timeout during read
        lastwarn(''); % reset lastwarn
        try
            a=fread(s,Fs*Ip_Data_Size,'char'); %read 1s data
            if(~isempty(lastwarn))
                error(lastwarn)
            end
        catch err
            disp('Timeout occurred, exiting'); 
            fclose(s); %close the serial port
            return; % Stop Script
        end
        %a=fread(s,Fs*Ip_Data_Size,'char'); %read 1s data
        while(index < length(a) - 4) % get 1s of ADC samples from the text data
            if(a(index) == '+')
                b=a(index+1:index+4,1); %reads the data from the serial port and stores it to the matrix a
               % a contains 4 ascii chars rep the hex no windowbuttondownfcn 
               a_dec = hex2dec(b);
               num(num_index) = 16*16*16*a_dec(1)+16*16*a_dec(2)+16*a_dec(3)+a_dec(4);
               num(num_index) = EcgFilter(b_lp, a_lp, num(num_index));
               %num(num_index) = MavgFilter(num(num_index));
               %num(num_index) = EcgNotchFilter(b_n, a_n, num(num_index));
               num_index = num_index + 1;
               index = index +4;
            end
            index = index +1;
        end
        temp = max(num);
        Y_Limit_Low = temp - 150;
        Y_Limit_High = temp + 150;
        ylim([Y_Limit_Low,Y_Limit_High]);
        
        plot(t+1:t+num_index-1,num(1,1:num_index-1));
        t=t+Fs;
        drawnow; %# Force the graphics to update immediately, if you dont do this plot will be executed only at teh end of the loop
    end
    global serial_exit ;
    if(serial_exit == 1)
        fclose(s); %close the serial port
        return; % Stop Script
    end
    n = n+1;
    t = 0;
end 
fclose(s); %close the serial port


% Callback subfunction defines two input arguments, click on the figure to
% execute the callback
function myCallback(src,eventdata) 
global serial_exit ;
disp('callback executed, stopping with this page'); 
serial_exit = 1; %Time to exit


function [data] = EcgFilter(b, a,value)

k = 1;
global dbuffer ;
while(k<length(b))
dbuffer(k) = dbuffer(k+1);
k=k+1;    
end
dbuffer(length(b)) = 0;
k = 1;
while(k<(length(b)+1))
dbuffer(k) = dbuffer(k) + value * b(k);
k=k+1;    
end

k = 1;
while(k<length(b))
dbuffer(k+1) = dbuffer(k+1) - dbuffer(1) * a(k+1);
k=k+1;    
end

data = dbuffer(1);

function [data] = EcgNotchFilter(b, a,value)

k = 1;
global nbuffer ;
while(k<length(b))
nbuffer(k) = nbuffer(k+1);
k=k+1;    
end
nbuffer(length(b)) = 0;
k = 1;
while(k<(length(b)+1))
nbuffer(k) = nbuffer(k) + value * b(k);
k=k+1;    
end

k = 1;
while(k<length(b))
nbuffer(k+1) = nbuffer(k+1) - nbuffer(1) * a(k+1);
k=k+1;    
end

data = nbuffer(1);

function [data] = MavgFilter(value)

k = 1;
global Dbuffer ;
global N;
% 1. shift by one element (discard the left most element)
while(k<N)
Dbuffer(k) = Dbuffer(k+1);
k=k+1;    
end
Dbuffer(N) = 0;

k = 1;
while(k<N+1)
Dbuffer(k) = Dbuffer(k) + value;
k=k+1;    
end

data = Dbuffer(1)/N;



