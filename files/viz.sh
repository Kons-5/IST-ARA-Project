#!/usr/bin/env bash
set -euo pipefail
if [[ $# -lt 2 ]]; then
    echo "Usage: $0 input.net output.png" >&2
    exit 1
fi
in="$1"
out="$2"

gawk '
function edge_label(t){if(t==1)return "customer";if(t==2)return "peer";if(t==3)return "provider";return "-"}
function edge_color(a,b){if(a==b){if(a==1)return "forestgreen";if(a==2)return "dodgerblue";if(a==3)return "crimson"}return "gray40"}
function key(a,b,x,y){x=(a<b)?a:b;y=(a<b)?b:a;return x "|" y}
function pad5(n){return sprintf("%05d",n)}
BEGIN{FS="[ \t]+"}
NF==3{
    u=$1+0;v=$2+0;t=$3+0
    if(u<0||u>65535||v<0||v>65535)next
    t_uv[u "|" v]=t
    nodes[u]=1;nodes[v]=1
    pair[key(u,v)]=1
}
END{
    print "digraph G {"
    print "  rankdir=TB;"
    print "  bgcolor=\"white\";"
    print "  nodesep=0.5; ranksep=1.0;"
    print "  splines=true;"
    print "  node [shape=circle, fontname=\"Sans\", style=filled, fillcolor=\"#f8f9fa\", penwidth=1.2];"
    print "  edge [fontname=\"Sans\", penwidth=1.8, color=\"#777777\", arrowsize=0.8, labeldistance=1.4, labelfontsize=10, labelangle=0];"
    for(n in nodes)printf("  \"%s\";\n",pad5(n))
    for(k in pair) {
        split(k,a,"|");u=a[1];v=a[2]
        t1=((u "|" v) in t_uv)?t_uv[u "|" v]:0
        t2=((v "|" u) in t_uv)?t_uv[v "|" u]:0
        if(t1==2||t2==2)printf("  { rank=same; \"%s\"; \"%s\"; }\n",pad5(u),pad5(v))
    }
    for(k in pair) {
        split(k,a,"|");u=a[1];v=a[2]
        t1=((u "|" v) in t_uv)?t_uv[u "|" v]:0
        t2=((v "|" u) in t_uv)?t_uv[v "|" u]:0
        col=edge_color(t1,t2)
        tlab=edge_label(t1)
        hlab=edge_label(t2)
        extra=(t1==2||t2==2)?", constraint=false":""
            printf("  \"%s\" -> \"%s\" [dir=both, arrowhead=normal, arrowtail=normal, taillabel=\"%s\", headlabel=\"%s\", color=\"%s\"%s];\n",pad5(u),pad5(v),tlab,hlab,col,extra)
    }
    print "}"
}
' "$in" | dot -Tpng -o "$out"
echo "wrote: $out"
