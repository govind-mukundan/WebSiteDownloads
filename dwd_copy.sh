!/bin/bash
# This script generates tar balls for specified folders and stores them in the given OP dir
# Reference --> http://www.linuxjournal.com/content/bash-arrays
# tar cvzf archive_name.tar.gz dirname/
# mv myfile.txt destination-directory array=($(ls -d */)) 
# ${string:position:length}

OPDIR="$HOME/Dropbox/Public/WebsiteDownloads/"
IPFOLDERS=("~/octopress/downloads/WebSiteDownloads/C/FatFsIntFiles/" "~/octopress/downloads/WebSiteDownloads/C/PICxi2cDriver/" "~/octopress/downloads/WebSiteDownloads/C/TFTDisplayDriver/HX8347G/" "~/octopress/downloads/WebSiteDownloads/C/TFTDisplayDriver/ILI9486L/" "~/octopress/downloads/WebSiteDownloads/C#/RobotSimulator_NET/" "~/octopress/downloads/WebSiteDownloads/Objective-C/RobotSimulator_iOS/" "~/octopress/downloads/WebSiteDownloads/Matlab/SerialPlot/" "~/octopress/downloads/WebSiteDownloads/C#/SharpGLWPFPlot/")

ipFolders=("$HOME/octopress/downloads/WebSiteDownloads/C/" "$HOME/octopress/downloads/WebSiteDownloads/C#/" "$HOME/octopress/downloads/WebSiteDownloads/Objective-C/"
"$HOME/octopress/downloads/WebSiteDownloads/Matlab/" "$HOME/octopress/downloads/WebSiteDownloads/C/TFTDisplayDriver/")


#ipFolders=("$HOME/octopress/downloads/WebSiteDownloads/C/")


echo "Total number of Folders: ${#ipFolders[*]}"

echo "Array items:"
for item in ${ipFolders[*]}
do
    cd "$item"
    subfolders=($(ls -d */)) #Get a list of sub folders. Will NOT work if there are special chars
    
    for folder in ${subfolders[*]}
    do
    printf "   %s: %s : %s\n" $item $folder ${folder%?}
    op=${folder%?}
    tar cvzf "${folder%?}"".tar.gz" "$folder"
    mv "${folder%?}"".tar.gz" $OPDIR
    done
    
done
