# ShidBooru
Minimal and extra shabby tag-based image system in c++ with qt, loads and saves plain sqlite files, minor opencv bindings (for webm/mp4 thumbnails), the aim is to have images on hand to copy to clipboard directly (for use on discord and stuff).

Does not cache or copy the files, basically just links tags to filepath, so if you remove a file on disk ShidBooru will ask you for the filepath again.

To create a new group of images or image and start tagging them, click "New booru" and then input path to a directory or a file (or use the "select folder" button).

Then double click on the miniature for the focused image (or press enter) then add or remove tags and click "OK" in the editor.

The editor can also play video (webm/mp4) files.

To copy the file to the clipboard, right click on the image directly in the group, or in the editor, you can also copy it to disk. The copy for videos copy the path to the clipboard, not the whole movie.

Filesearch by tag has the following options :
- AND (default) : "zebra shell" returns files that have both tags
- OR : "~ambiguous ~join" returns files that have either of those tags
- EXCLUDE : "rock -cape" returns files that are tagged "rock" but not "cape"
- WILDCARD : "b*re" returns files that are tagged "bare" and "bore" etc