<h1>File System Manager Utility</h1>
<p>This is a command-line utility for managing files and directories.</p>
<h2>Key Commands</h2>
<ul>
  <li>
    <strong>r:</strong> Rename a file
  </li>
  <li>
    <strong>c:</strong> Copy files
  </li>
  <li>
    <strong>d:</strong> Delete files
  </li>
  <li>
    <strong>m:</strong> Move files
  </li>
  <li>
    <strong>a:</strong> Calculate CRC (Cyclic Redundancy Check)
  </li>
  <li>
    <strong>s:</strong> Sort files
  </li>
  <li>
    <strong>f:</strong> Find files
  </li>
</ul>
<h2>Functionality</h2>
<ul>
  <li>Get list of files in a directory</li>
  <li>Sort files in alphabetical order</li>
  <li>Read file content</li>
  <li>Rename a file</li>
  <li>Delete file or directory</li>
  <li>Copy files</li>
  <li>Move file to another location</li>
  <li>Calculate total size of directory and subdirectories</li>
  <li>Display full file information</li>
  <li>Calculate CRC for files</li>
</ul>
<h2>Downloading from GitHub</h2>
<p>To download this project from GitHub onto your machine:</p>
<ol>
  <li>Clone the repository using Git:</li>
  <pre>
		<code>git clone https://github.com/Hannah-Kaliada/KALIADA_OSISP_prj_2024.git</code>
	</pre>
  <li>Navigate to the project directory:</li>
  <pre>
		<code>cd KALIADA_OSISP_prj_2024</code>
	</pre>
  <li>Compile and run the program as described in the <strong>Compilation</strong> and <strong>Usage</strong> sections above. </li>
</ol>
<h2>Configuration</h2>
<p>Before compiling and running the program, make sure to configure the settings in the <code>config.h</code> file. </p>
<h2>Compilation</h2>
<p>To compile the program, use the provided <code>makefile</code>. Simply run the following command: </p>
<pre>
	<code>make</code>
</pre>
<h2>Usage</h2>
<p>After compiling, you can run the program by executing the generated executable.</p>
<pre>
	<code>./main</code>
</pre>
<p>You can also use <code>make sudo</code> to compile and execute the program with elevated privileges: </p>
<pre>
	<code>make sudo</code>
</pre>
 <h2>Dependencies</h2>
    <p>This program requires the ncurses library for terminal UI. The `pkg-config` tool is used to manage this dependency in the makefile.</p>
    <p>Before compiling, make sure you have `ncurses` installed on your system. You can install it using the package manager of your Linux distribution:</p>
    <pre><code>sudo apt-get install libncurses5-dev</code></pre>
    <p>Or:</p>
    <pre><code>sudo yum install ncurses-devel</code></pre>
<h2>Development Environment Details</h2>
<ul>
  <li>Operating System: macOS Sonoma 14.4.1 (Build 23E224)</li>
  <li>Development Tools: Xcode</li>
  <li>Programming Language: C (ISO/IEC 9899:2011)</li>
  <li>File System Type: APFS</li>
</ul>
